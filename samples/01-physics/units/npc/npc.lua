-- Note: the following table must be global and uniquely named.
Npc = Npc or {
	data = {}
}

local data = Npc.data

local CONFIG = {
	speed = 1.5,
	gravity = 6 * 9.8,
	max_fall_speed = 50.0,
	min_step_distance = 2.75,
	max_step_distance = 5.0,
	move_time_min = 0.75,
	move_time_max = 2.0,
	idle_time_min = 0.4,
	idle_time_max = 1.1,
	max_wander_distance = 10.0
}

local TWO_PI = math.pi * 2.0
local MAX_WANDER_DISTANCE_SQUARED = CONFIG.max_wander_distance * CONFIG.max_wander_distance

local function random_range(min_value, max_value)
	return min_value + math.random() * (max_value - min_value)
end

local function begin_idle(state)
	state.state = "idle"
	state.move_time = 0
	state.wait_time = random_range(CONFIG.idle_time_min, CONFIG.idle_time_max)
end

local function choose_target(state, current_position)
	local spawn_position = state.spawn_position:unbox()
	local target_position = nil

	for _ = 1, 8 do
		local angle = math.random() * TWO_PI
		local distance = random_range(CONFIG.min_step_distance, CONFIG.max_step_distance)
		local offset = Vector3(math.cos(angle) * distance, math.sin(angle) * distance, 0)
		local candidate = current_position + offset
		local spawn_offset = candidate - spawn_position

		if spawn_offset.x * spawn_offset.x + spawn_offset.y * spawn_offset.y <= MAX_WANDER_DISTANCE_SQUARED then
			target_position = Vector3(candidate.x, candidate.y, current_position.z)
			break
		end
	end

	if target_position == nil then
		local to_spawn = spawn_position - current_position
		to_spawn.z = 0

		if Vector3.length(to_spawn) <= 0.001 then
			begin_idle(state)
			return
		end

		Vector3.normalize(to_spawn)
		local distance = math.min(CONFIG.max_step_distance, Vector3.distance(current_position, spawn_position))
		target_position = current_position + to_spawn * distance
		target_position.z = current_position.z
	end

	state.target_position:store(target_position)
	state.state = "moving"
	state.move_time = random_range(CONFIG.move_time_min, CONFIG.move_time_max)
end

function Npc.spawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		world_data = {}
		data[world] = world_data
	end

	local physics_world = World.physics_world(world)
	local scene_graph = World.scene_graph(world)

	for _, unit in pairs(units) do
		if world_data[unit] == nil then
			local mover = PhysicsWorld.mover_instance(physics_world, unit)
			local transform = SceneGraph.instance(scene_graph, unit)
			local position = SceneGraph.local_position(scene_graph, transform)

			PhysicsWorld.mover_set_position(physics_world, mover, position)

			world_data[unit] = {
				mover = mover,
				spawn_position = Vector3Box(position),
				target_position = Vector3Box(position),
				state = "idle",
				move_time = 0,
				vertical_speed = -0.1,
				wait_time = random_range(CONFIG.idle_time_min, CONFIG.idle_time_max)
			}
		end
	end
end

function Npc.update(world, dt)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	local physics_world = World.physics_world(world)
	local scene_graph = World.scene_graph(world)

	for unit, state in pairs(world_data) do
		local mover = state.mover
		local current_position = PhysicsWorld.mover_position(physics_world, mover)
		local move_delta = Vector3.zero()

		if state.state == "idle" then
			state.wait_time = state.wait_time - dt

			if state.wait_time <= 0 then
				choose_target(state, current_position)
			end
		else
			local target_position = state.target_position:unbox()
			local to_target = target_position - current_position
			to_target.z = 0
			state.move_time = state.move_time - dt

			if Vector3.length(to_target) <= CONFIG.speed * dt then
				move_delta = to_target
				begin_idle(state)
			elseif state.move_time <= 0 then
				begin_idle(state)
			else
				Vector3.normalize(to_target)
				move_delta = to_target * (CONFIG.speed * dt)
			end
		end

		if PhysicsWorld.mover_collides_down(physics_world, mover) then
			state.vertical_speed = -0.1
		else
			state.vertical_speed = math.max(-CONFIG.max_fall_speed, state.vertical_speed - CONFIG.gravity * dt)
		end

		move_delta.z = state.vertical_speed * dt
		PhysicsWorld.mover_move(physics_world, mover, move_delta)

		local transform = SceneGraph.instance(scene_graph, unit)
		SceneGraph.set_local_position(scene_graph, transform, PhysicsWorld.mover_position(physics_world, mover))
		if Game and Game.mover_debug_line then
			PhysicsWorld.mover_debug_draw(physics_world, mover, Game.mover_debug_line, Color4(255, 210, 80, 255))
		end
	end
end

function Npc.unspawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	for _, unit in pairs(units) do
		world_data[unit] = nil
	end
end

return Npc
