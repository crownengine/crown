-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

local Behavior = Behavior or {}

function Behavior.spawned(world, units)
	for uu = 1, #units do
		local unit = units[uu]

		-- Hide the mesh.
		local render_world = World.render_world(world)
		local camera_mesh = RenderWorld.mesh_instance(render_world, unit)
		if camera_mesh ~= nil then
			RenderWorld.mesh_set_visible(render_world, camera_mesh, false)
		end
	end
end

function Behavior.update(world, dt)
end

function Behavior.unspawned(world, units)
end

return Behavior
