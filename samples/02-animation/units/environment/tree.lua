local Behavior = Behavior or {}
local Data = Data or {}

function Behavior.spawned(world, units)
	if Data[world] == nil then
		Data[world] = {}
	end

	for uu = 1, #units do
		local unit = units[uu]

		if Data[world][unit] == nil then
			-- Store instance-specific data
			-- Data[world][unit] = {}
		end

		-- Set sprite depth based on unit's position
		local sg = World.scene_graph(world)
		local rw = World.render_world(world)
		local tr = SceneGraph.instance(sg, unit)
		local pos = SceneGraph.local_position(sg, tr)
		local depth = math.floor(1000 + (1000 - 32*pos.z))
		local sprite = RenderWorld.sprite_instance(rw, unit)
		RenderWorld.sprite_set_depth(rw, sprite, depth)
	end
end

function Behavior.unspawned(world, units)
	-- Cleanup
	for uu = 1, #units do
		if Data[world][units] then
			Data[world][units] = nil
		end
	end
end

function Behavior.update(world, dt)
	-- Do nothing
end

return Behavior
