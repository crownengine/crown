-- Note: the following table must be global and uniquely named.
UnitsMovingPlatformMovingPlatform = UnitsMovingPlatformMovingPlatform or {
	data = {}
}

local data = UnitsMovingPlatformMovingPlatform.data

function UnitsMovingPlatformMovingPlatform.spawned(world, units)
	if data[world] == nil then
		data[world] = {}
	end

	local world_data = data[world]

	for _, unit in pairs(units) do
		-- Store instance-specific data.
		if world_data[unit] == nil then
			world_data[unit] = { zmin = 0.75, zmax = 7.75, zvel = 2, twait = 1.0, tcur = 0 }
		end
	end
end

function UnitsMovingPlatformMovingPlatform.update(world, dt)
	local world_data = data[world]
	local scene_graph = World.scene_graph(world)

	-- Moving platform.
	for unit, unit_data in pairs(world_data) do
		local platform = SceneGraph.instance(scene_graph, unit)
		local platform_pos = SceneGraph.local_position(scene_graph, platform)

		if unit_data.tcur == 0 and (platform_pos.z >= unit_data.zmax or platform_pos.z <= unit_data.zmin) then
			unit_data.zvel = -unit_data.zvel
			unit_data.tcur = unit_data.twait
		end

		if unit_data.tcur <= 0 then
			local new_position = platform_pos + Vector3(0, 0, unit_data.zvel) * dt
			new_position.z = math.max(unit_data.zmin, math.min(unit_data.zmax, new_position.z))
			SceneGraph.set_local_position(scene_graph, platform, new_position)
			unit_data.tcur = 0
		else
			unit_data.tcur = unit_data.tcur - dt
		end
	end
end

function UnitsMovingPlatformMovingPlatform.unspawned(world, units)
	local world_data = data[world]

	-- Cleanup.
	for _, unit in pairs(units) do
		if world_data[unit] then
			world_data[unit] = nil
		end
	end
end

return UnitsMovingPlatformMovingPlatform
