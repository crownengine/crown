-- Note: the following table must be global and uniquely named.
UnitsRotatingPlatformRotatingPlatform = UnitsRotatingPlatformRotatingPlatform or {
	data = {}
}

local data = UnitsRotatingPlatformRotatingPlatform.data

local ROTATION_CONFIG = {
	rotating_platform_x = { axis = "x", speed = math.pi * 0.5 },
	rotating_platform_y = { axis = "y", speed = math.pi * 0.5 },
	rotating_platform_z = { axis = "z", speed = math.pi * 0.5 }
}

local function rotation_config_for_unit(world, unit)
	for unit_name, config in pairs(ROTATION_CONFIG) do
		if World.unit_by_name(world, unit_name) == unit then
			return config
		end
	end

	return ROTATION_CONFIG.rotating_platform_z
end

function UnitsRotatingPlatformRotatingPlatform.spawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		world_data = {}
		data[world] = world_data
	end

	local scene_graph = World.scene_graph(world)

	for _, unit in pairs(units) do
		if world_data[unit] == nil then
			local transform = SceneGraph.instance(scene_graph, unit)
			local config = rotation_config_for_unit(world, unit)

			world_data[unit] = {
				transform = transform,
				base_rotation = QuaternionBox(SceneGraph.local_rotation(scene_graph, transform)),
				angle = 0,
				axis = config.axis,
				speed = config.speed
			}
		end
	end
end

function UnitsRotatingPlatformRotatingPlatform.update(world, dt)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	local scene_graph = World.scene_graph(world)

	for _, state in pairs(world_data) do
		state.angle = state.angle + state.speed * dt

		local rotation_axis = state.axis == "x"
			and Vector3(1, 0, 0)
			or state.axis == "y"
			and Vector3(0, 1, 0)
			or Vector3(0, 0, 1)
		local rotation = Quaternion.multiply(state.base_rotation:unbox(), Quaternion(rotation_axis, state.angle))
		SceneGraph.set_local_rotation(scene_graph, state.transform, rotation)
	end
end

function UnitsRotatingPlatformRotatingPlatform.unspawned(world, units)
	local world_data = data[world]
	if world_data == nil then
		return
	end

	for _, unit in pairs(units) do
		world_data[unit] = nil
	end
end

return UnitsRotatingPlatformRotatingPlatform
