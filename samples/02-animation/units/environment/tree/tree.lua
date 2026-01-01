-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

-- Note: the following table must be global and uniquely named.
Tree = Tree or {
	data = {}
}

local data = Tree.data

-- Called after units are spawned into a world.
function Tree.spawned(world, units)
	if data[world] == nil then
		data[world] = {}
	end

	local world_data = data[world]

	for _, unit in pairs(units) do
		-- Store instance-specific data.
		if world_data[unit] == nil then
			world_data[unit] = {}
		end

		-- Set sprite depth based on unit's position.
		local sg = World.scene_graph(world)
		local rw = World.render_world(world)
		local tr = SceneGraph.instance(sg, unit)
		local pos = SceneGraph.local_position(sg, tr)
		local depth = math.floor(1000 + (1000 - 32*pos.y))
		local sprite = RenderWorld.sprite_instance(rw, unit)
		RenderWorld.sprite_set_depth(rw, sprite, depth)
	end
end

-- Called once per frame.
function Tree.update(world, dt)
	local world_data = data[world]

	for unit, unit_data in pairs(world_data) do
		-- Update unit.
	end
end

-- Called before units are unspawned from a world.
function Tree.unspawned(world, units)
	local world_data = data[world]

	-- Cleanup.
	for _, unit in pairs(units) do
		if world_data[unit] then
			world_data[unit] = nil
		end
	end
end

return Tree
