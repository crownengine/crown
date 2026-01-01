-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

require "core/game/camera"

Game = Game or {
	physics_world = nil,
	render_world = nil,
	scene_graph = nil,
	camera = nil,
	world_gui = nil,
	cursor_disabled = false,

	-- Player movement.
	character_unit = nil,
	gravity = 6*9.8,
	vertical_speed = 0,
	walk_speed = 8,
	run_multiplier = 1.8,

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

function Game.level_loaded()
	Game.physics_world = World.physics_world(GameBase.world)
	Game.render_world = World.render_world(GameBase.world)
	Game.scene_graph = World.scene_graph(GameBase.world)
	Game.camera = FPSCamera(GameBase.world, GameBase.camera_unit)
	Game.world_gui = World.create_world_gui(GameBase.world)

	-- Create character controller.
	Game.character_unit = World.unit_by_name(GameBase.world, "character")

	-- Link camera to mover.
	if Game.character_unit then
		local character_transform = SceneGraph.instance(Game.scene_graph, Game.character_unit)
		local camera_transform    = SceneGraph.instance(Game.scene_graph, GameBase.camera_unit)

		if SceneGraph.parent(Game.scene_graph, camera_transform) == nil then
			SceneGraph.link(Game.scene_graph, character_transform, camera_transform)
		end
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
	-- Toggle cursor state and quit logic.
	if Keyboard.any_pressed() then
		if Keyboard.pressed(Keyboard.button_id("escape")) then
			if Game.cursor_disabled then
				Window.set_cursor_mode("normal")
				Game.cursor_disabled = false
			else
				Device.quit()
			end
		else
			Window.set_cursor_mode("disabled")
			Game.cursor_disabled = true
		end
	end

	local camera_world = Game.camera:world_pose()
	World.set_listener_pose(GameBase.world, camera_world)

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
	if Mouse.pressed(Mouse.button_id("left")) then
		local tr = SceneGraph.instance(Game.scene_graph, Game.camera:unit())
		local pos = SceneGraph.world_position(Game.scene_graph, tr)
		local dir = Matrix4x4.y(SceneGraph.local_pose(Game.scene_graph, tr))
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
	if Keyboard.pressed(Keyboard.button_id("e")) then
		local camera_transform = SceneGraph.instance(Game.scene_graph, Game.camera:unit())
		local camera_position = SceneGraph.world_position(Game.scene_graph, camera_transform)
		local camera_forward = Matrix4x4.y(SceneGraph.world_pose(Game.scene_graph, camera_transform))
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

	local dx = Keyboard.button(Keyboard.button_id("d")) - Keyboard.button(Keyboard.button_id("a"))
	local dy = Keyboard.button(Keyboard.button_id("w")) - Keyboard.button(Keyboard.button_id("s"))

	if Game.character_unit then
		local mover = PhysicsWorld.mover_instance(Game.physics_world, Game.character_unit)

		-- Player mover.
		local coll_sides = PhysicsWorld.mover_collides_sides(Game.physics_world, mover)
		local coll_up    = PhysicsWorld.mover_collides_up(Game.physics_world, mover)
		local coll_down  = PhysicsWorld.mover_collides_down(Game.physics_world, mover)

		local camera_transform = SceneGraph.instance(Game.scene_graph, Game.camera:unit())
		local camera_forward = Matrix4x4.y(SceneGraph.local_pose(Game.scene_graph, camera_transform))
		local camera_right = Matrix4x4.x(SceneGraph.local_pose(Game.scene_graph, camera_transform))
		local delta = Vector3(camera_forward.x, camera_forward.y, 0) * dy
			+ Vector3(camera_right.x, camera_right.y, 0) * dx

		if Vector3.length(delta) > 0.0001 then
			Vector3.normalize(delta)
		end

		local jump_pressed = Keyboard.pressed(Keyboard.button_id("space"))
		local run_pressed = Keyboard.button(Keyboard.button_id("shift_left"))

		local speed = Game.walk_speed
		if run_pressed ~= 0 then
			speed = speed * Game.run_multiplier
		end
		delta = delta * speed

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
		PhysicsWorld.mover_move(Game.physics_world, mover, delta*(1/120))

		-- Copy mover position to character position.
		local mover_pos = PhysicsWorld.mover_position(Game.physics_world, mover)
		local character_transform = SceneGraph.instance(Game.scene_graph, Game.character_unit)
		assert(character_transform)
		SceneGraph.set_local_position(Game.scene_graph, character_transform, mover_pos)
	else
		Game.camera:move(dt, dx, dy)
	end

	-- Update camera.
	if Game.cursor_disabled then
		local cursor_delta = Mouse.axis(Mouse.axis_id("cursor_delta"))
		Game.camera:rotate(dt, cursor_delta.x, cursor_delta.y)
	end

	-- Toggle help.
	if Keyboard.pressed(Keyboard.button_id("f1")) then
		GameBase.show_help = not GameBase.show_help
	end

	-- Draw 3D labels.
	local camera_view = Matrix4x4.y(camera_world)
	local camera_right = Matrix4x4.x(camera_world)

	for k, v in pairs({ label_slopes = "Slopes", label_moving_platforms = "Moving Platforms", label_trigger = "Trigger", label_raycast = "Raycast" }) do
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

	GameBase.draw_help({{ key = "f1", desc = "Toggle help" },
		{ key = "w/a/s/d", desc = "Walk" },
		{ key = "shift", desc = "Run" },
		{ key = "space", desc = "Jump" },
		{ key = "e", desc = "Interact" },
		{ key = "left click", desc = "Shoot" },
		{ key = "z", desc = "Toggle physics debug" },
		{ key = "x", desc = "Toggle graphics debug" },
		{ key = "esc", desc = "Quit or Enable cursor" }}
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
end
