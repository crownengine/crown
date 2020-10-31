require "core/editors/level_editor/camera"

UnitPreview = UnitPreview or {}

function UnitPreview:init()
	self._world = Device.create_world()
	self._sg = World.scene_graph(self._world)
	self._rw = World.render_world(self._world)
	self._pw = World.physics_world(self._world)
	self._camera = Camera(self._world, World.spawn_unit(self._world, "core/units/camera"))
	self._unit_name = nil
	self._unit_id = nil

	World.spawn_unit(self._world, "core/units/light", Vector3(1000, 1000, -1000))
end

function UnitPreview:update(dt)
	World.update(self._world, dt)

	if self._unit_id then
		local mesh = RenderWorld.mesh_instance(self._rw, self._unit_id)
		local tm, hext = mesh
			and RenderWorld.mesh_obb(self._rw, mesh)
			or Matrix4x4.identity(), Vector3(1, 1, 1)

		local radius = Vector3.length(hext)
		radius = math.ceil(radius / 2)  * 2
		radius = radius <   1 and   1 or radius
		radius = radius > 100 and 100 or radius

		local camera_unit = self._camera:unit()
		local pos = Vector3(radius, radius, -radius) * 2
		local camera_pos = Matrix4x4.translation(tm) + pos
		local target_pos = Matrix4x4.translation(tm)
		local tr = SceneGraph.instance(self._sg, camera_unit)
		SceneGraph.set_local_rotation(self._sg, tr, Quaternion.look(Vector3.normalize(target_pos - camera_pos)))
		SceneGraph.set_local_position(self._sg, tr, camera_pos)
	end

	self._camera:update(dt, 0, 0, {})
end

function UnitPreview:render(dt)
	Device.render(self._world, self._camera:unit())
end

function UnitPreview:shutdown()
	Device.destroy_world(self._world)
end

function UnitPreview:set_preview_resource(type, name)
	local unit = nil
	if type == "unit" then
		unit = name
	elseif type == "sound" then
		unit = "core/units/sound"
	else
		return
	end

	if self._unit_name == unit then
		return
	end

	if self._unit_id then
		World.destroy_unit(self._world, self._unit_id)
	end

	self._unit_name = unit
	self._unit_id = World.spawn_unit(self._world, unit)

	local actor = PhysicsWorld.actor_instance(self._pw, self._unit_id)
	if actor then
		PhysicsWorld.actor_set_kinematic(self._pw, actor, true)
	end
end
