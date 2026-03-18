-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/camera"

Game = Game or {
	physics_world = nil,
	render_world = nil,
	scene_graph = nil,
	camera = nil,
	world_gui = nil,
	mover_debug_line = nil,
	cursor_disabled = false,
	last_input_is_pad = false,

	-- Player movement.
	character_unit = nil,
	gravity = 6*9.8,
	vertical_speed = 0,
	walk_speed = 3,
	run_multiplier = 4,
	crouch_multiplier = 0.45,
	crouch_camera_drop = 0.6,
	mover_stand_height = 1.8,
	mover_crouch_height = 1.0,
	mover_crouching = false,
	character_support_actor = nil,
	character_support_prev_pose = Matrix4x4Box(),
	character_base_rotation = QuaternionBox(),

	-- Camera management.
	first_person_camera_local_offset = Vector3Box(),
	first_person_height = 2.6,
	third_person_camera = true,
	third_person_distance = 8,
	third_person_height = 1.4,
	pad_look_speed = 32,

	-- Bullets management.
	max_bullets = 20,
	num_bullets = 0,
	bullets_head = 1,
	bullets_tail = 1,
	bullets = {},
	ages = {},

	-- Actionable door.
	door_open = false,

	-- Debug.
	debug_graphics = false,
	debug_physics = false,
}

GameBase.game = Game
GameBase.game_level = "levels/mover"
GameBase.show_help = true
GameBase.show_build = true

local function k(bind)
	if Game.last_input_is_pad then
		if bind == "f1" then return "start"
		elseif bind == "w/a/s/d" then return "left thumb"
		elseif bind == "mouse" then return "right thumb"
		elseif bind == "shift" then return "B"
		elseif bind == "ctrl" then return "LB"
		elseif bind == "space" then return "A"
		elseif bind == "e" then return "Y"
		elseif bind == "-" then return "right thumb"
		elseif bind == "left click" then return "RB" end
	end

	return bind
end

function Game.level_loaded()
	Game.physics_world = World.physics_world(GameBase.world)
	Game.render_world = World.render_world(GameBase.world)
	Game.scene_graph = World.scene_graph(GameBase.world)
	Game.camera = FPSCamera(GameBase.world, GameBase.camera_unit)
	Game.world_gui = World.create_world_gui(GameBase.world)
	Game.mover_debug_line = World.create_debug_line(GameBase.world, true)

	-- Create character controller.
	Game.character_unit = World.unit_by_name(GameBase.world, "character")
	Game.character_base_rotation:store(Quaternion.identity())
	local asm = World.animation_state_machine(GameBase.world)
	if Game.character_unit and asm then
		local animation = AnimationStateMachine.instance(asm, Game.character_unit)
		if animation then
			AnimationStateMachine.trigger(asm, animation, "idle")
		end
	end

	-- Keep camera unparented so it is unaffected by any character parent transform.
	if Game.character_unit then
		local character_transform = SceneGraph.instance(Game.scene_graph, Game.character_unit)
		local camera_transform = SceneGraph.instance(Game.scene_graph, GameBase.camera_unit)
		local camera_parent = SceneGraph.parent(Game.scene_graph, camera_transform)
		local fp_local_offset = camera_parent == character_transform
			and SceneGraph.local_position(Game.scene_graph, camera_transform)
			or Vector3.zero()

		if camera_parent then
			local camera_world_pose = SceneGraph.world_pose(Game.scene_graph, camera_transform)
			SceneGraph.unlink(Game.scene_graph, camera_transform)
			SceneGraph.set_local_pose(Game.scene_graph, camera_transform, camera_world_pose)
		end

		-- Preserve the authored mesh orientation so yaw can be applied on top of it.
		Game.character_base_rotation:store(SceneGraph.local_rotation(Game.scene_graph, character_transform))

		Game.character_support_actor = nil
		Game.character_support_prev_pose:store(Matrix4x4.identity())
		Game.first_person_camera_local_offset:store(fp_local_offset)
	end

	-- Create a pool of reusable bullets.
	for i=1, Game.max_bullets do
		local bullet_unit = World.spawn_unit(GameBase.world, "units/bullet/bullet", Vector3(0, 0, -100))
		local bullet_actor = PhysicsWorld.actor_instance(Game.physics_world, bullet_unit)

		-- Avoid overlapping bullets interaction.
		PhysicsWorld.actor_disable_gravity(Game.physics_world, bullet_actor)
		PhysicsWorld.actor_disable_collision(Game.physics_world, bullet_actor)

		Game.bullets[i] = bullet_unit
		Game.ages[i] = 0
	end

	-- Debug.
	PhysicsWorld.enable_debug_drawing(Game.physics_world, Game.debug_physics)
	RenderWorld.enable_debug_drawing(Game.render_world, Game.debug_graphics)
end

function Game.update(dt)
	-- Track the last active input device for show_help labels.
	local keyboard_used = Keyboard.any_pressed() ~= nil
	local pad_used = Pad1.any_pressed() ~= nil
	if not pad_used then
		local pad_left = Pad1.axis(Pad1.axis_id("left"))
		local pad_right = Pad1.axis(Pad1.axis_id("right"))
		pad_used = pad_left.x ~= 0
			or pad_left.y ~= 0
			or pad_right.x ~= 0
			or pad_right.y ~= 0
	end

	if pad_used and not keyboard_used then
		Game.last_input_is_pad = true
	elseif keyboard_used and not pad_used then
		Game.last_input_is_pad = false
	end

	-- Toggle cursor state and quit logic.
	if Keyboard.any_pressed() then
		if Keyboard.pressed(Keyboard.button_id("escape")) then
			if Game.cursor_disabled then
				Game.cursor_disabled = not Window.set_cursor_mode("normal")
			else
				Device.quit()
			end
		else
			Game.cursor_disabled = Window.set_cursor_mode("disabled")
		end
	end

	-- Toggle camera mode.
	if Keyboard.released(Keyboard.button_id("minus"))
		or Pad1.pressed(Pad1.button_id("thumb_right")) then
		Game.third_person_camera = not Game.third_person_camera
	end

	-- Recycle used bullets after they expire.
	local head = Game.bullets_head
	for i=1, Game.num_bullets do
		if Game.ages[head] < 4 then
			Game.ages[head] = Game.ages[head] + dt
			head = head % Game.max_bullets + 1
		else
			local bullet_unit = Game.bullets[Game.bullets_head]
			local bullet_actor = PhysicsWorld.actor_instance(Game.physics_world, bullet_unit)

			-- Avoid overlapping bullets interaction.
			PhysicsWorld.actor_disable_gravity(Game.physics_world, bullet_actor)
			PhysicsWorld.actor_disable_collision(Game.physics_world, bullet_actor)

			-- Bullets may have been recycled while still in motion.
			PhysicsWorld.actor_set_linear_velocity(Game.physics_world, bullet_actor, Vector3.zero())
			PhysicsWorld.actor_set_angular_velocity(Game.physics_world, bullet_actor, Vector3.zero())

			-- Teleport bullet somewhere player can't see it.
			PhysicsWorld.actor_teleport_world_position(Game.physics_world, bullet_actor, Vector3(0, 0, -100))

			Game.ages[Game.bullets_head] = 0
			Game.bullets_head = Game.bullets_head % Game.max_bullets + 1
			Game.num_bullets = Game.num_bullets - 1
		end
	end

	-- Shoot a bullet when left mouse button is pressed.
	local shoot_pressed = Mouse.pressed(Mouse.button_id("left"))
		or Pad1.pressed(Pad1.button_id("shoulder_right"))
	if shoot_pressed then
		local camera_pose = Game.camera:world_pose()
		local pos = Matrix4x4.translation(camera_pose)
		local dir = Matrix4x4.y(camera_pose)
		Vector3.normalize(dir)

		if Game.num_bullets == Game.max_bullets then
			-- Empty magazine. Play a sad sound maybe.
		else
			local bullet_unit = Game.bullets[Game.bullets_tail]
			local bullet_actor = PhysicsWorld.actor_instance(Game.physics_world, bullet_unit)

			-- Gravity and collision is disabled when the bullet gets recycled. Re-enable both.
			PhysicsWorld.actor_enable_gravity(Game.physics_world, bullet_actor)
			PhysicsWorld.actor_enable_collision(Game.physics_world, bullet_actor)

			-- Move the bullet in front of player and shoot it forward.
			PhysicsWorld.actor_teleport_world_position(Game.physics_world, bullet_actor, pos + dir * 1.5)
			PhysicsWorld.actor_add_impulse(Game.physics_world, bullet_actor, dir * 50.0)

			World.play_sound(GameBase.world, "sfx/shoot", false, 0.6, 150.0)

			Game.bullets_tail = Game.bullets_tail % Game.max_bullets + 1
			Game.num_bullets = Game.num_bullets + 1
		end
	end

	-- Actionable door.
	local interact_pressed = Keyboard.pressed(Keyboard.button_id("e"))
		or Pad1.pressed(Pad1.button_id("y"))
	if interact_pressed then
		local camera_pose = Game.camera:world_pose()
		local camera_position = Matrix4x4.translation(camera_pose)
		local camera_forward = Matrix4x4.y(camera_pose)
		Vector3.normalize(camera_forward)
		local hit, hit_pos, normal, time, unit, actor = PhysicsWorld.cast_ray(Game.physics_world, camera_position, camera_forward, 100)

		if hit then
			local door_button = World.unit_by_name(GameBase.world, "raycast_door_button")

			-- If we hit the door button and we are not too far, open/close the door.
			if unit == door_button and Vector3.distance(camera_position, hit_pos) < 10 then
				local door = World.unit_by_name(GameBase.world, "raycast_door")
				local door_transform = SceneGraph.instance(Game.scene_graph, door)
				local door_position = SceneGraph.local_position(Game.scene_graph, door_transform)
				local door_dx = 2.3
				local new_door_position = door_position + Vector3(Game.door_open and door_dx or -door_dx, 0, 0)

				SceneGraph.set_local_position(Game.scene_graph, door_transform, new_door_position)
				Game.door_open = not Game.door_open
			end
		end
	end

	-- Mouse/keyboard controls.
	local move_dx = Keyboard.button(Keyboard.button_id("d")) - Keyboard.button(Keyboard.button_id("a"))
	local move_dy = Keyboard.button(Keyboard.button_id("w")) - Keyboard.button(Keyboard.button_id("s"))
	local look_dx = 0
	local look_dy = 0
	if Game.cursor_disabled then
		local cursor_delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
		look_dx = look_dx + cursor_delta.x
		look_dy = look_dy + cursor_delta.y
	end

	-- Joypad controls.
	local pad_left = Pad1.axis(Pad1.axis_id("left"))
	local pad_right = Pad1.axis(Pad1.axis_id("right"))
	move_dx = move_dx + pad_left.x
	move_dy = move_dy + pad_left.y
	look_dx = look_dx + pad_right.x * Game.pad_look_speed
	look_dy = look_dy - pad_right.y * Game.pad_look_speed

	-- Rotate camera.
	if math.abs(look_dx) > 0.0001 or math.abs(look_dy) > 0.0001 then
		Game.camera:rotate(dt, look_dx, look_dy)
	end

	if Game.character_unit then
		local mover = PhysicsWorld.mover_instance(Game.physics_world, Game.character_unit)
		local character_transform = SceneGraph.instance(Game.scene_graph, Game.character_unit)
		assert(character_transform)

		-- Player mover.
		local coll_sides = PhysicsWorld.mover_collides_sides(Game.physics_world, mover)
		local coll_up    = PhysicsWorld.mover_collides_up(Game.physics_world, mover)
		local coll_down  = PhysicsWorld.mover_collides_down(Game.physics_world, mover)

		-- Moving platforms character interaction.
		local mover_carry_delta = Vector3.zero()
		if coll_down then
			local support_actor = PhysicsWorld.mover_actor_colliding_down(Game.physics_world, mover)
			if support_actor ~= nil and PhysicsWorld.actor_is_kinematic(Game.physics_world, support_actor) then
				local support_pose = PhysicsWorld.actor_world_pose(Game.physics_world, support_actor)
				if Game.character_support_actor ~= nil and Game.character_support_actor == support_actor then
					local support_delta = Matrix4x4.multiply(Matrix4x4.invert(Game.character_support_prev_pose:unbox()), support_pose)
					local mover_pos = PhysicsWorld.mover_position(Game.physics_world, mover)
					local mover_center = PhysicsWorld.mover_center(Game.physics_world, mover)
					local current_mover_origin = mover_pos + mover_center
					local carried_mover_origin = Matrix4x4.transform(support_delta, current_mover_origin)
					mover_carry_delta = carried_mover_origin - current_mover_origin
				end

				Game.character_support_actor = support_actor
				Game.character_support_prev_pose:store(support_pose)
			else
				Game.character_support_actor = nil
			end
		else
			Game.character_support_actor = nil
		end

		local camera_pose = Game.camera:world_pose()
		local camera_forward = Matrix4x4.y(camera_pose)
		local camera_right = Matrix4x4.x(camera_pose)
		local camera_transform = SceneGraph.instance(Game.scene_graph, Game.camera:unit())
		local delta = Vector3(camera_forward.x, camera_forward.y, 0) * move_dy
			+ Vector3(camera_right.x, camera_right.y, 0) * move_dx

		if Vector3.length(delta) > 0.0001 then
			Vector3.normalize(delta)
		end

		local jump_pressed = Keyboard.pressed(Keyboard.button_id("space"))
			or Pad1.pressed(Pad1.button_id("a"))
		local run_pressed = Keyboard.button(Keyboard.button_id("shift_left")) ~= 0
			or Pad1.button(Pad1.button_id("b")) ~= 0
		local crouch_pressed = Keyboard.button(Keyboard.button_id("ctrl_left")) ~= 0
			or Keyboard.button(Keyboard.button_id("ctrl_right")) ~= 0
			or Pad1.button(Pad1.button_id("shoulder_left")) ~= 0
		local crouch_active = crouch_pressed

		-- Prevent uncrouching while blocked overhead.
		if not crouch_active and Game.mover_crouching and coll_up then
			crouch_active = true
		end

		-- Resize capsule only when crouch state changes.
		if crouch_active ~= Game.mover_crouching then
			local current_center = PhysicsWorld.mover_center(Game.physics_world, mover)
			local center_dz = (Game.mover_stand_height - Game.mover_crouch_height) * 0.5
			Game.mover_crouching = crouch_active
			if Game.mover_crouching then
				PhysicsWorld.mover_set_height(Game.physics_world, mover, Game.mover_crouch_height)
				PhysicsWorld.mover_set_center(Game.physics_world, mover, Vector3(current_center.x, current_center.y, current_center.z - center_dz))
			else
				PhysicsWorld.mover_set_height(Game.physics_world, mover, Game.mover_stand_height)
				PhysicsWorld.mover_set_center(Game.physics_world, mover, Vector3(current_center.x, current_center.y, current_center.z + center_dz))
			end
		end

		local speed = Game.walk_speed
		if run_pressed then
			speed = speed * Game.run_multiplier
		end
		if crouch_active then
			speed = speed * Game.crouch_multiplier
		end
		delta = delta * speed

		-- Animation.
		local move_speed = Vector3.length(delta)
		local animation_moving = move_speed > 0.0001
		local animation_running = animation_moving and run_pressed and not crouch_active

		if animation_moving then
			local yaw = math.atan2(delta.y, delta.x) + math.pi * 0.5
			local character_rotation = Quaternion.multiply(Quaternion.from_axis_angle(Vector3(0, 0, 1), yaw), Game.character_base_rotation:unbox())
			SceneGraph.set_local_rotation(Game.scene_graph, character_transform, character_rotation)
		end

		local asm = World.animation_state_machine(GameBase.world)
		if asm then
			local animation = AnimationStateMachine.instance(asm, Game.character_unit)
			local speed_id  = AnimationStateMachine.variable_id(asm, animation, "speed")

			AnimationStateMachine.set_variable(asm, animation, speed_id, math.min(1.0, move_speed / Game.walk_speed))

			if animation_running then
				AnimationStateMachine.trigger(asm, animation, "run")
			elseif animation_moving then
				AnimationStateMachine.trigger(asm, animation, "walk")
			else
				AnimationStateMachine.trigger(asm, animation, "idle")
			end
		end

		if jump_pressed and coll_down then
			Game.vertical_speed = Game.gravity * 0.25
		else
			if coll_down then
				Game.vertical_speed = -0.1
			else
				Game.vertical_speed = math.max(-50, Game.vertical_speed - Game.gravity * dt)
			end
		end

		delta.z = Game.vertical_speed
		if Vector3.length(mover_carry_delta) > 0.0001 then
			local mover_pos = PhysicsWorld.mover_position(Game.physics_world, mover)
			PhysicsWorld.mover_set_position(Game.physics_world, mover, mover_pos + mover_carry_delta)
		end
		PhysicsWorld.mover_move(Game.physics_world, mover, delta*dt)

		-- Copy mover position to character position.
		local mover_pos = PhysicsWorld.mover_position(Game.physics_world, mover)
		SceneGraph.set_local_position(Game.scene_graph, character_transform, mover_pos)
		local character_world_pose = SceneGraph.world_pose(Game.scene_graph, character_transform)
		local character_world_position = Matrix4x4.translation(character_world_pose)

		if Game.third_person_camera then
			local camera_pos = character_world_position
				+ Vector3(0, 0, Game.third_person_height)
				- camera_forward * Game.third_person_distance
			SceneGraph.set_local_position(Game.scene_graph, camera_transform, camera_pos)
		else
			local fp_offset = Game.first_person_camera_local_offset:unbox()
			local char_right = Matrix4x4.x(character_world_pose)
			local char_forward = Matrix4x4.y(character_world_pose)
			if Vector3.length(char_right) > 0.0001 then Vector3.normalize(char_right) end
			if Vector3.length(char_forward) > 0.0001 then Vector3.normalize(char_forward) end
			local height = Game.first_person_height - (Game.mover_crouching and Game.crouch_camera_drop or 0)
			local camera_pos = character_world_position
				+ char_right * fp_offset.x
				+ char_forward * fp_offset.y
				+ Vector3(0, 0, height)
			SceneGraph.set_local_position(Game.scene_graph, camera_transform, camera_pos)
		end
	else
		Game.camera:move(dt, move_dx, move_dy)
	end

	local camera_world = Game.camera:world_pose()
	World.set_listener_pose(GameBase.world, camera_world)

	DebugLine.submit(Game.mover_debug_line)
	DebugLine.reset(Game.mover_debug_line)

	-- Toggle help.
	if Keyboard.pressed(Keyboard.button_id("f1"))
		or Pad1.pressed(Pad1.button_id("start")) then
		GameBase.show_help = not GameBase.show_help
	end

	-- Draw 3D labels.
	local camera_view = Matrix4x4.y(camera_world)
	local camera_right = Matrix4x4.x(camera_world)

	for k, v in pairs({ label_slopes = "Slopes"
		, label_movers = "Movers"
		, label_irregular_terrain = "Irregular terrain"
		, label_stairs = "Stairs"
		, label_crouching = "Crouching"
		, label_moving_platforms = "Moving platforms"
		, label_rotating_platforms = "Rotating platforms"
		, label_trigger = "Trigger"
		, label_raycast = "Raycast" }) do
		local label_unit = World.unit_by_name(GameBase.world, k)

		if label_unit then
			local label_transform = SceneGraph.instance(Game.scene_graph, label_unit)
			local label_position = SceneGraph.local_position(Game.scene_graph, label_transform)
			local font_resource = "core/game/hud/debug"
			local font_size = 1.5
			local label_extents = Gui.text_extents(Game.world_gui, font_size, v, font_resource)
			local label_pose = Matrix4x4.from_axes(camera_right
				, Vector3(0, 0, 1)
				, camera_view
				, label_position - camera_right * label_extents.x * 0.5
				)

			Gui.text_3d(Game.world_gui, label_pose, Vector3.zero(), font_size, v, font_resource)
		end
	end

	-- Draw crosshair.
	local win_w, win_h = Device.resolution()
	Gui.rect(GameBase.screen_gui, Vector2(win_w/2, win_h/2), Vector2(4, 4), Color4(255, 0, 255, 255))

	GameBase.draw_help({{ key = k("f1"), desc = "Toggle help" },
		{ key = k("w/a/s/d"), desc = "Walk" },
		{ key = k("mouse"), desc = "Look" },
		{ key = k("shift"), desc = "Run" },
		{ key = k("ctrl"), desc = "Crouch" },
		{ key = k("space"), desc = "Jump" },
		{ key = k("e"), desc = "Interact" },
		{ key = k("-"), desc = "Toggle 1st/3rd person" },
		{ key = k("left click"), desc = "Shoot" },
		{ key = k("z"), desc = "Toggle physics debug" },
		{ key = k("x"), desc = "Toggle graphics debug" },
		{ key = k("esc"), desc = "Quit or Enable cursor" }}
		, "Crown Physics Sample"
		)

	-- Toggle debug drawing.
	if Keyboard.released(Keyboard.button_id("z")) then
		Game.debug_physics = not Game.debug_physics
		PhysicsWorld.enable_debug_drawing(Game.physics_world, Game.debug_physics)
	end

	if Keyboard.released(Keyboard.button_id("x")) then
		Game.debug_graphics = not Game.debug_graphics
		RenderWorld.enable_debug_drawing(Game.render_world, Game.debug_graphics)
	end
end

function Game.render(dt)
end

function Game.shutdown()
	World.destroy_debug_line(GameBase.world, Game.mover_debug_line)
end
