require "core/level_editor/camera"

UnitPreview = UnitPreview or {}

function UnitPreview:init()
	self._world = Device.create_world()
	self._physics_world = World.physics_world(self._world)
	self._sg = World.scene_graph(self._world)
	self._camera_unit = World.spawn_unit(self._world, "core/units/camera")
	self._fpscamera = FPSCamera(self._world, self._camera_unit)
	self._unit = nil
	self._rotation = 0

	-- Spawn camera
	local camera_transform = SceneGraph.transform_instances(self._sg, self._camera_unit)
	local pos = Vector3(5, 5, -5)
	local dir = Vector3.normalize(Vector3.zero() - pos)
	SceneGraph.set_local_rotation(self._sg, camera_transform, Quaternion.look(dir))
	SceneGraph.set_local_position(self._sg, camera_transform, pos)

	World.spawn_unit(self._world, "core/units/light", Vector3(1000, 1000, -1000))
end

function UnitPreview:update(dt)
	World.update(self._world, dt)

	if self._unit then
		local tr = SceneGraph.transform_instances(self._sg, self._unit)
		if tr then
			SceneGraph.set_local_rotation(self._sg, tr, Quaternion(Vector3(0, 1, 0), self._rotation))
		end
	end

	self._rotation = self._rotation + dt

	self._fpscamera:update(0, 0, {})
end

function UnitPreview:render(dt)
	-- Update window's viewport
	local win_w, win_h = Device.resolution()
	local aspect = win_w / win_h
	local camera = World.camera(self._world, self._camera_unit)
	World.set_camera_aspect(self._world, camera, win_w/win_h)

	Device.render(self._world, self._fpscamera:camera())
end

function UnitPreview:shutdown()
	Device.destroy_world(self._world)
end

function UnitPreview:set_preview_unit(unit)
	if self._unit then
		World.destroy_unit(self._world, self._unit)
	end

	self._rotation = 0
	self._unit = World.spawn_unit(self._world, unit)

	local actor = PhysicsWorld.actor_instances(self._physics_world, self._unit)
	if actor then
		PhysicsWorld.set_actor_kinematic(self._physics_world, actor, true)
	end
end
