-- Copyright (c) 2012-2025 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT

Tree = Tree or {
	data = {}
}

local data = Tree.data

function Tree.spawned(units, world)
	if data[world] == nil then
		data[world] = {}
	end

	for uu = 1, #units do
		local unit = units[uu]

		if data[world][unit] == nil then
			-- Store instance-specific data.
			-- data[world][unit] = {}
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

function Tree.unspawned(units, world)
	-- Cleanup.
	for uu = 1, #units do
		if data[world][units] then
			data[world][units] = nil
		end
	end
end

function Tree.update(world, dt)
	-- Do nothing.
end

return Tree
