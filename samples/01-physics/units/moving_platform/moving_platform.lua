-- Note: the following table must be global and uniquely named.
UnitsMovingPlatformMovingPlatform = UnitsMovingPlatformMovingPlatform or {
	data = {}
}

local data = UnitsMovingPlatformMovingPlatform.data

local PLATFORM_MOVEMENT = {
	moving_platform_x = { axis = "x", distance = 16.0, direction = 1 },
	moving_platform_y = { axis = "y", distance = 16.0, direction = 1 },
	moving_platform_z = { axis = "z", distance = 7.0, direction = 1 },
	moving_platform_zneg = { axis = "z", distance = 7.0, direction = -1 },
	-- Backward-compatible names used by the sample level.
	moving_platform_z1 = { axis = "z", distance = 7.0, direction = 1 },
	moving_platform_z2 = { axis = "z", distance = 7.0, direction = -1 }
}

local function platform_movement_for_unit(world, unit)
	for unit_name, movement in pairs(PLATFORM_MOVEMENT) do
		if World.unit_by_name(world, unit_name) == unit then
			return movement
		end
	end

	-- Default to positive z movement if the unit name does not match.
	return PLATFORM_MOVEMENT.moving_platform_z
end

function UnitsMovingPlatformMovingPlatform.spawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		world_data = {}
		data[world] = world_data
	end

	for _, unit in pairs(units) do
		world_data[unit] = world_data[unit] or { wait_duration = 1.0, wait_time = 0, speed = 2 }
	end
end

function UnitsMovingPlatformMovingPlatform.update(world, dt)
	local world_data = data[world]
	local scene_graph = World.scene_graph(world)

	for unit, state in pairs(world_data) do
		local transform = SceneGraph.instance(scene_graph, unit)
		local position = SceneGraph.local_position(scene_graph, transform)

		if state.axis == nil then
			local movement = platform_movement_for_unit(world, unit)
			local start = position[movement.axis]

			state.axis = movement.axis
			state.min_position = movement.direction > 0 and start or start - movement.distance
			state.max_position = movement.direction > 0 and start + movement.distance or start
			state.velocity = movement.direction * state.speed
		end

		local axis = state.axis
		if state.wait_time == 0 then
			local position_on_axis = position[axis]

			if position_on_axis >= state.max_position or position_on_axis <= state.min_position then
				state.velocity = -state.velocity
				state.wait_time = state.wait_duration
			end
		end

		if state.wait_time <= 0 then
			local delta = state.velocity * dt
			local next_position = position + Vector3(
				axis == "x" and delta or 0,
				axis == "y" and delta or 0,
				axis == "z" and delta or 0
			)

			if axis == "x" then
				next_position.x = math.max(state.min_position, math.min(state.max_position, next_position.x))
			elseif axis == "y" then
				next_position.y = math.max(state.min_position, math.min(state.max_position, next_position.y))
			else
				next_position.z = math.max(state.min_position, math.min(state.max_position, next_position.z))
			end

			SceneGraph.set_local_position(scene_graph, transform, next_position)
			state.wait_time = 0
		else
			state.wait_time = state.wait_time - dt
		end
	end
end

function UnitsMovingPlatformMovingPlatform.unspawned(world, units)
	local world_data = data[world]

	for _, unit in pairs(units) do
		world_data[unit] = nil
	end
end

return UnitsMovingPlatformMovingPlatform
