local Behavior = Behavior or {}
local Data = Data or {}

function Behavior.spawned(world, units)
	if Data[world] == nil then
		Data[world] = {}
	end

	for uu = 1, #units do
		local unit = units[uu]

		-- Store instance-specific data
		if Data[world][unit] == nil then
			-- Data[world][unit] = {}
		end

		-- Hide the camera mesh
		local render_world = World.render_world(world)
		local camera_mesh = RenderWorld.mesh_instance(render_world, unit)
		RenderWorld.mesh_set_visible(render_world, camera_mesh, false)
	end
end

function Behavior.update(world, dt)
	-- Update all units
end

function Behavior.unspawned(world, units)
	-- Cleanup
	for uu = 1, #units do
		if Data[world][units] then
			Data[world][units] = nil
		end
	end
end

return Behavior
