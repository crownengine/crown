=================
Lua API Reference
=================

Math
====

Vector3
-------

Constructors
~~~~~~~~~~~~

**Vector3** (x, y, z) : Vector3
	Returns a new vector from individual elements.

Functions
~~~~~~~~~

**x** (v) : float
	Returns the x value of the vector.

**y** (v) : float
	Returns the y value of the vector.

**z** (v) : float
	Returns the z value of the vector.

**.x** : float
	Returns/assigns the x value of the vector.

**.y** : float
	Returns/assigns the y value of the vector.

**.z** : float
	Returns/assigns the z value of the vector.

**set_x** (v, x)
	Sets the value of the x value of the vector.

**set_y** (v, y)
	Sets the value of the y value of the vector.

**set_z** (v, z)
	Sets the value of the z value of the vector.

**elements** (v) : float, float, float
	Returns the x, y and z elements of the vector.

**add** (a, b) : Vector3
	Adds the vector *a* to *b* and returns the result.

**subtract** (a, b) : Vector3
	Subtracts the vector *b* from *a* and returns the result.

**multiply** (a, k) : Vector3
	Multiplies the vector *a* by the scalar *k* and returns the result.

**dot** (a, b) : float
	Returns the dot product between the vectors *a* and *b*.

**cross** (a, b) : Vector3
	Returns the cross product between the vectors *a* and *b*.

**equal** (a, b) : bool
	Returns true whether the vectors *a* and *b* are equal.

**length** (a) : float
	Returns the lenght of *a*.

**length_squared** (a) : float
	Returns the squared length of *a*.

**set_length** (a, len)
	Sets the lenght of *a* to *len*.

**normalize** (a) : Vector3
	Normalizes *a* and returns the result.

**distance** (a, b) : float
	Returns the distance between the points *a* and *b*.

**distance_squared** (a, b) : float
	Returns the squared distance between the points *a* and *b*.

**angle** (a, b) : float
	Returns the angle between the vectors *a* and *b*.

**max** (a, b) : Vector3
	Returns a vector that contains the largest value for each element from *a* and *b*.

**min** (a, b) : Vector3
	Returns a vector that contains the smallest value for each element from *a* and *b*.

**lerp** (a, b, t) : Vector3
	Returns the linearly interpolated vector between *a* and *b* at time *t* in [0, 1].

| **forward** () : Vector3
| **backward** () : Vector3
| **left** () : Vector3
| **right** () : Vector3
| **up** () : Vector3
| **down** () : Vector3
| 	Returns the corresponding semantic axis.

**zero** () : Vector3
	Returns a vector with all values set to zero.

**to_string** (v) : string
	Returns a string representing the vector *v*.

Vector3Box
----------

Constructors
~~~~~~~~~~~~

**Vector3Box** () : Vector3Box
	Returns a new Vector3Box initialized with the zero vector.

**Vector3Box** (v) : Vector3Box
	Returns a new Vector3Box from the Vector3 *v*.

**Vector3Box** (x, y, z) : Vector3Box
	Returns a new Vector3Box from individual elements.

Functions
~~~~~~~~~

**store** (v)
	Stores the Vector3 *v* in the box.

**store** (x, y, z)
	Stores Vector3(x, y, z) in the box.

**unbox** () : Vector3
	Returns the stored vector from the box.

Quaternion
----------

Constructors
~~~~~~~~~~~~

**Quaternion** (axis, angle) : Quaternion
	Returns a new quaternion from *axis* and *angle*.

**from_elements** (x, y, z, w) : Quaternion
	Returns a new quaternion from individual elements.

**from_axis_angle** (axis, angle) : Quaternion
	Returns a new quaternion from *axis* and *angle*.

Functions
~~~~~~~~~

**negate** (q) : Quaternion
	Negates the quaternion *q* and returns the result.

**identity** () : Quaternion
	Returns the identity quaternion.

**multiply** (a, b) : Quaternion
	Multiplies the quaternions *a* and *b*. (i.e. rotates first by *a* then by *b*).

**multiply_by_scalar** (a, k) : Quaternion
	Multiplies the quaternion *a* by the scalar *k*.

**dot** (a, b) : float
	Returns the dot product between quaternions *a* and *b*.

**length** (q) : float
	Returns the length of *q*.

**normalize** (q) : Quaternion
	Normalizes the quaternion *q* and returns the result.

**conjugate** (q) : Quaternion
	Returns the conjugate of quaternion *q*.

**inverse** (q) : Quaternion
	Returns the inverse of quaternion *q*.

**power** (q, exp) : Quaternion
	Returns the quaternion *q* raised to the power of *exp*.

**elements** (q) : float, float, float, float
	Returns the x, y, z and w elements of the quaternion.

**look** (dir, [up]) : Quaternion
	Returns the quaternion describing the rotation needed to face towards *dir*.
	If *up* is not specified, Vector3.up() is used.

**right** (q) : Vector3
	Returns the right axis of the rotation described by *q*.

**up** (q) : Vector3
	Returns the up axis of the rotation described by *q*.

**forward** (q) : Vector3
	Returns the forward axis of the rotation described by *q*.

**lerp** (a, b, t) : Quaternion
	Returns the linearly interpolated quaternion between *a* and *b* at time *t* in [0, 1].
	It uses NLerp.

**to_string** (q) : string
	Returns a string representing the quaternion *q*.

QuaternionBox
-------------

Constructors
~~~~~~~~~~~~

**QuaternionBox** () : QuaternionBox
	Returns a new QuaternionBox initialized with the identity quaternion.

**QuaternionBox** (q) : QuaternionBox
	Returns a new QuaternionBox from the Quaternion *q*.

**QuaternionBox** (x, y, z, w) : QuaternionBox
	Returns a new QuaternionBox from individual elements.

Functions
~~~~~~~~~

**store** (q)
	Stores the Quaternion *q* in the box.

**store** (x, y, z, w)
	Stores Quaternion(x, y, z, w) in the box.

**unbox** () : Quaternion
	Returns the stored quaternion from the box.

Matrix4x4
---------

Constructors
~~~~~~~~~~~~

**Matrix4x4** (xx, xy, xz, xw, yx, yy, yz, yw, zx, zy, zz, zw, tx, ty, tz, tw) : Matrix4x4
	Returns a new matrix from individual elements.

**from_quaternion** (q) : Matrix4x4
	Returns a new matrix from *q*.

**from_translation** (t) : Matrix4x4
	Returns a new matrix from *t*.

**from_quaternion_translation** (q, t) : Matrix4x4
	Returns a new matrix from *q* and *t*.

**from_axes** (x, y, z, t) : Matrix4x4
	Returns a new matrix from *x*, *y*, *z* and *t*.

Functions
~~~~~~~~~

**copy** (m) : Matrix4x4
	Returns a copy of the matrix *m*.

**add** (a, b) : Matrix4x4
	Adds the matrix *a* to *b* and returns the result.

**subtract** (a, b) : Matrix4x4
	Subtracts the matrix *b* from *a* and returns the result.

**multiply** (a, b) : Matrix4x4
	Multiplies the matrix *a* by *b* and returns the result. (i.e. transforms first by *a* then by *b*)

**transpose** (m) : Matrix4x4
	Transposes the matrix *m* and returns the result.

**determinant** (m) : float
	Returns the determinant of the matrix *m*.

**invert** (m) : Matrix4x4
	Inverts the matrix *m* and returns the result.

**x** (m) : Vector3
	Returns the x axis of the matrix *m*.

**y** (m) : Vector3
	Returns the y axis of the matrix *m*.

**z** (m) : Vector3
	Returns the z axis of the matrix *m*.

**set_x** (m, x)
	Sets the x axis of the matrix *m*.

**set_y** (m, y)
	Sets the y axis of the matrix *m*.

**set_z** (m, z)
	Sets the z axis of the matrix *m*.

**rotation** (m) : Quaternion
	Returns the rotation portion of the matrix *m*.

**set_rotation** (m, r)
	Sets the rotation portion of the matrix *m*.

**translation** (m, a) : Vector3
	Returns the translation portion of the matrix *m*.

**set_translation** (m, t)
	Sets the translation portion of the matrix *m*.

**identity** ()
	Returns the identity matrix.

**transform** (m, v) : Vector3
	Transforms the vector *v* by the matrix *m* and returns the result.

**to_string** (m) : string
	Returns a string representing the matrix *m*.

Matrix4x4Box
------------

Constructors
~~~~~~~~~~~~

**Matrix4x4Box** () : Matrix4x4Box
	Returns a new Matrix4x4Box initialized with the identity matrix.

**Matrix4x4Box** (m) : Matrix4x4Box
	Returns a new Matrix4x4Box from the Matrix4x4 *m*.

Functions
~~~~~~~~~

**store** (m)
	Stores the Matrix4x4 *m* in the box.

**unbox** () : Matrix4x4
	Returns the stored matrix from the box.

Color4
------

Constructors
~~~~~~~~~~~~

**Color4** (r, g, b, a) : Color4
	Returns a new Color4 from individual elements.

Functions
~~~~~~~~~

**lerp** (a, b, t) : Color4
	Returns the linearly interpolated color between *a* and *b* at time *t* in [0, 1].

| **black** () : Color4
| **white** () : Color4
| **red** () : Color4
| **green** () : Color4
| **blue** () : Color4
| **yellow** () : Color4
| **orange** () : Color4
|	Returns the corresponding mnemonic color.

**to_string** (c) : string
	Returns a string representing the color *c*.

Math
----

**ray_plane_intersection** (from, dir, point, normal) : float
	Returns the distance along ray (from, dir) to intersection point with plane defined by
	*point* and *normal* or -1.0 if no intersection.

**ray_disc_intersection** (from, dir, center, radius, normal) : float
	Returns the distance along ray (from, dir) to intersection point with disc defined by
	*center*, *radius* and *normal* or -1.0 if no intersection.

**ray_sphere_intersection** (from, dir, center, radius) : float
	Returns the distance along ray (from, dir) to intersection point with sphere defined by
	*center* and *radius* or -1.0 if no intersection.

**ray_obb_intersection** (from, dir, tm, half_extents) : float
	Returns the distance along ray (from, dir) to intersection point with the oriented
	bounding box (tm, half_extents) or -1.0 if no intersection.

**ray_triangle_intersection** (from, dir, v0, v1, v2) : float
	Returns the distance along ray (from, dir) to intersection point with the triangle
	(v0, v1, v2) or -1.0 if no intersection.

UnitManager
===========

**create** ([world]) : UnitId
	Creates a new empty unit. If *world* is specified, the unit will be owned by
	that world.

**destroy** (unit)
	Destroys the given *unit*.

**alive** (unit) : bool
	Returns whether the unit is alive.

World
=====

**spawn_unit** (world, name, [position, rotation]) : UnitId
	Spawns a new instance of the unit *name* at the given *position* and *rotation*.

**spawn_empty_unit** (world) : UnitId
	Spawns a new empty unit and returns its id.

**destroy_unit** (world, unit)
	Destroys the given *unit*.

**num_units** (world) : int
	Returns the number of units in the *world*.

**units** (world) : table
	Returns all the the units in the world in a table.

**update_animations** (world, dt)
	Update all animations with *dt*.

**update_scene** (world, dt)
	Updates the scene with *dt*.

**update** (world, dt)
	Updates the world with *dt*.

**create_debug_line** (world, depth_test) : DebugLine
	Creates a new DebugLine. *depth_test* controls whether to
	enable depth test when rendering the lines.

**destroy_debug_line** (world, line)
	Destroys the debug *line*.

**scene_graph** (world) : SceneGraph
	Returns the scene graph.

**render_world** (world) : RenderWorld
	Returns the render sub-world.

**physics_world** (world) : PhysicsWorld
	Returns the physics sub-world.

**sound_world** (world) : SoundWorld
	Returns the sound sub-world.

Camera
------

**camera_create** (world, unit, projection, fov, far_range, near_range, pose) : Id
	Creates a new camera for *unit* and returns its id.
	Projection can be either ``orthographic`` or ``perspective``.

**camera_instances** (world, unit) : Id
	Returns the IDs for all the cameras of the *unit*.

**camera_set_projection_type** (world, camera, projection)
	Sets the projection type of the camera.
	Projection can be either ``orthographic`` or ``perspective``.

**camera_projection_type** (world, camera) : string
	Returns the projection type of the camera.
	It can be either ``orthographic`` or ``perspective``.

**camera_fov** (world, camera) : float
	Returns the field-of-view of the camera in degrees.

**camera_set_fov** (world, camera, fov)
	Sets the field-of-view of the camera in degrees.

**camera_near_clip_distance** (world, camera) : float
	Returns the near clip distance of the camera.

**camera_set_near_clip_distance** (world, camera, near)
	Sets the near clip distance of the camera.

**camera_far_clip_distance** (world, camera) : float
	Returns the far clip distance of the camera.

**camera_set_far_clip_distance** (world, camera, far)
	Sets the far clip distance of the camera.

**camera_set_orthographic_metrics** (world, camera, left, right, bottom, top)
	Sets the coordinates for orthographic clipping planes. (Orthographic projection only.)

**camera_screen_to_world** (world, camera, pos) : Vector3
	Returns *pos* from screen-space to world-space coordinates.

**camera_world_to_screen** (world, camera, pos) : Vector3
	Returns *pos* from world-space to screen-space coordinates.

Sound
-----

**play_sound** (world, name, [loop, volume, position, range]) : SoundInstanceId
	Plays the sound with the given *name* at the given *position*, with the given
	*volume* and *range*. *loop* controls whether the sound must loop or not.

**stop_sound** (world, id)
	Stops the sound with the given *id*.

**link_sound** (world, id, unit, node)
	Links the sound *id* to the *node* of the given *unit*.
	After this call, the sound *id* will follow the unit *unit*.

**set_listener_pose** (world, pose)
	Sets the *pose* of the listener.

**set_sound_position** (world, id, position)
	Sets the *position* of the sound *id*.

**set_sound_range** (world, id, range)
	Sets the *range* of the sound *id*.

**set_sound_volume** (world, id, volume)
	Sets the *volume* of the sound *id*.

Level
-----

**load_level** (world, name, [pos, rot]) : Level
	Loads the level *name* into the world at the given *position* and *rotation*.

SceneGraph
==========

**create** (sg, unit, position, rotation, scale) : Id
	Creates the transform for the *unit* and returns its ID.

**destroy** (sg, unit, id)
	Destroys the transform for the *unit*. The transform *id* is ignored.

**instances** (sg, unit) : Id
	Returns the IDs for all the transforms of the *unit*.

**local_position** (sg, unit) : Vector3
	Returns the local position of the *unit*.

**local_rotation** (sg, unit) : Quaternion
	Returns the local rotation of the *unit*.

**local_scale** (sg, unit) : Vector3
	Returns the local scale of the *unit*.

**local_pose** (sg, unit) : Matrix4x4
	Returns the local pose of the *unit*.

**world_position** (sg, unit) : Vector3
	Returns the world position of the *unit*.

**world_rotation** (sg, unit) : Quaternion
	Returns the world rotation of the *unit*.

**world_pose** (sg, unit) : Matrix4x4
	Returns the world pose of the *unit*.

**set_local_position** (sg, unit, position)
	Sets the local *position* of the *unit*.

**set_local_rotation** (sg, unit, rotation)
	Sets the local *rotation* of the *unit*.

**set_local_scale** (sg, unit, scale)
	Sets the local *scale* of the *unit*.

**set_local_pose** (sg, unit, pose)
	Sets the local *pose* of the *unit*.

**link** (sg, child, parent)
	Links the unit *child* to the unit *parent*.

**unlink** (sg, unit)
	Unlinks the *unit* from its parent if it has any.
	After unlinking, the @a unit's local pose is set to its previous world pose.

Material
========

**set_float** (material, name, value)
	Sets the *value* of the variable *name*.

**set_vector2** (material, name, value)
	Sets the *value* of the variable *name*.

**set_vector3** (material, name, value)
	Sets the *value* of the variable *name*.

RenderWorld
===========

**enable_debug_drawing** (rw, enable)
	Sets whether to *enable* debug drawing.

Mesh
----

**mesh_create** (rw, unit, mesh_resource, geometry_name, material_resource, visible, pose) : Id
	Creates a new mesh instance for *unit* and returns its id.

**mesh_destroy** (rw, id)
	Destroys the mesh *id*.

**mesh_instances** (rw, unit) : Id
	Returns the IDs for all the meshes of the *unit*.

**mesh_raycast** (rw, id, from, dir) : float
	Returns the distance along ray (from, dir) to intersection point with the mesh *id* or -1.0 if no intersection.

Sprite
------

**sprite_create** (rw, unit, sprite_resource, material_resource, visible, pose) : Id
	Creates a new sprite instance for the *unit* and returns its id.

**sprite_destroy** (rw, unit)
	Destroys the sprite of the *unit*.

**sprite_instances** (rw, unit) : Id
	Returns the IDs for all the sprites of the *unit*.

**sprite_set_frame** (rw, unit, index)
	Sets the frame *index* of the sprite.

**sprite_set_visible** (rw, unit, visible)
	Sets whether the sprite is *visible*.

**sprite_flip_x** (rw, unit, flip)
	Sets whether to flip the sprite on the x-axis.

**sprite_flip_y** (rw, unit, flip)
	Sets whether to flip the sprite on the y-axis.

Light
-----

**light_create** (rw, unit, type, range, intensity, spot_angle, color, pose) : Id
	Creates a new light for the *unit* and returns its id.
	Type can be either ``directional``, ``omni`` or ``spot``.

**light_destroy** (rw, unit)
	Destroys the light of the *unit*.

**light_instances** (rw, unit) : Id
	Returns the IDs for all the lights of the *unit*.

**light_type** (rw, unit) : string
	Returns the type of the light of the *unit*.
	It can be either ``directional``, ``omni`` or ``spot``.

**light_color** (rw, unit) : Color4
	Returns the color of the light.

**light_range** (rw, unit) : float
	Returns the range of the light.

**light_intensity** (rw, unit) : float
	Returns the intensity of the light.

**light_spot_angle** (rw, unit) : float
	Returns the spot angle of the light.

**light_set_type** (rw, unit, type)
	Sets the *type* of the light.

**light_set_color** (rw, unit, color)
	Sets the *color* of the light.

**light_set_range** (rw, unit, range)
	Sets the *range* of the light.

**light_set_intensity** (rw, unit, intensity)
	Sets the *intensity* of the light.

**light_set_spot_angle** (rw, unit, angle)v
	Sets the spot *angle* of the light.

**light_debug_draw** (rw, unit, debug_line)
	Fills *debug_line* with debug lines from the light.

PhysicsWorld
=============

**gravity** (pw) : Vector3
	Returns the gravity.

**set_gravity** (pw, gravity)
	Sets the gravity.

**raycast** (pw, from, dir, length, mode) : table
	Returns the actors which intersects the raycast.
	Mode can be either ``closest`` or ``all``.

**enable_debug_drawing** (pw, enable)
	Sets whether to *enable* debug drawing.

Actor
-----

**actor_instances** (pw, unit) : Id
	Returns the IDs for all the actors of the *unit*.

**actor_world_position** (pw, actor) : Vector3
	Returns the world position of the actor.

**actor_world_rotation** (pw, actor) : Quaternion
	Returns the world rotation of the actor.

**actor_world_pose** (pw, actor) : Matrix4x4
	Returns the world pose of the actor.

**actor_teleport_world_position** (pw, actor, position)
	Teleports the actor to the given world position.

**actor_teleport_world_rotation** (pw, actor, rotation)
	Teleports the actor to the given world rotation.

**actor_teleport_world_pose** (pw, actor, pose)
	Teleports the actor to the given world pose.

**actor_center_of_mass** (pw, actor) : Vector3
	Returns the center of mass of the actor.

**actor_enable_gravity** (pw, actor)
	Enables gravity for the actor.

**actor_disable_gravity** (pw, actor)
	Disables gravity for the actor.

**actor_enable_collision** (pw, actor)
	Enables collision detection for the actor.

**actor_disable_collision** (pw, actor)
	Disables collision detection for the actor.

**actor_set_collision_filter** (pw, actor, name)
	Sets the collision filter of the actor.

**actor_set_kinematic** (pw, actor, kinematic)
	Sets whether the actor is kinematic or not.
	Note that this call has no effect on static actors.

**actor_move** (pw, actor, position)
	Moves the actor to *pos*
	Note that this call only affects nonkinematic actors.

**actor_is_static** (pw, actor) : bool
	Returns whether the actor is static.

**actor_is_dynamic** (pw, actor) bool
	Returns whether the actor is dynamic.

**actor_is_kinematic** (pw, actor) : bool
	Returns whether the actor is kinematic (keyframed).

**actor_is_nonkinematic** (pw, actor) : bool
	Returns whether the actor is nonkinematic (i.e. dynamic and not kinematic).

**actor_linear_damping** (pw, actor) : float
	Returns the linear damping of the actor.

**actor_set_linear_damping** (pw, actor, damping)
	Sets the linear damping of the actor.

**actor_angular_damping** (pw, actor) : float
	Returns the angular damping of the actor.

**actor_set_angular_damping** (pw, actor, rate)
	Sets the angular damping of the actor.

**actor_linear_velocity** (pw, actor) : Vector3
	Returns the linear velocity of the actor.

**actor_set_linear_velocity** (pw, actor, velocity)
	Sets the linear velocity of the actor.
	Note that this call only affects nonkinematic actors.

**actor_angular_velocity** (pw, actor) : Vector3
	Returns the angular velocity of the actor.

**actor_set_angular_velocity** (pw, actor, velocity)
	Sets the angular velocity of the actor.
	Note that this call only affects nonkinematic actors.

**actor_add_impulse** (pw, actor, impulse)
	Adds a linear impulse (acting along the center of mass) to the actor.
	Note that this call only affects nonkinematic actors.

**actor_add_impulse_at** (pw, actor, impulse, position)
	Adds a linear impulse (acting along the world position *pos*) to the actor.
	Note that this call only affects nonkinematic actors.

**actor_add_torque_impulse** (pw, actor, impulse)
	Adds a torque impulse to the actor.

**actor_push** (pw, actor, velocity, mass)
	Pushes the actor as if it was hit by a point object with the given *mass*
	travelling at the given *velocity*.
	Note that this call only affects nonkinematic actors.

**actor_push_at** (pw, actor, velocity, mass, position)
	Like push() but applies the force at the world position *pos*.
	Note that this call only affects nonkinematic actors.

**actor_is_sleeping** (pw, actor) : bool
	Returns whether the actor is sleeping.

**actor_wake_up** (pw, actor)
	Wakes the actor up.

SoundWorld
===========

**stop_all** (sound_world)
	Stops all the sounds in the world.

**pause_all** (sound_world)
	Pauses all the sounds in the world

**resume_all** (sound_world)
	Resumes all previously paused sounds in the world.

**is_playing** (sound_world, id) : bool
	Returns whether the sound *id* is playing.

ResourcePackage
================

**load** (package)
	Loads all the resources in the package.
	Note that the resources are not immediately available after the call is made,
	instead, you have to poll for completion with has_loaded().

**unload** (package)
	Unloads all the resources in the package.

**flush** (package)
	Waits until the package has been loaded.

**has_loaded** (package) : bool
	Returns whether the package has been loaded.

Device
======

**argv** () : table
	Returns a table containing the command line parameters the engine was started with.

**platform** () : string
	Returns a string identifying what platform the engine is running on.
	It can be either ``android``, ``linux`` or ``windows``

**architecture** () : string
	Returns a string identifying what architecture the engine is running on.
	It can be either ``32-bit`` or ``64-bit``.

**version** () : string
	Returns a string identifying the engine version.
	The form is "major.minor.micro".

**last_delta_time** () : float
	Returns the time in seconds needed to render the last frame

**quit** ()
	Quits the application.

**resolution** () : float, float
	Returns the main window resolution (width, height).

**create_world** () : World
	Creates a new world.

**destroy_world** (world)
	Destroys the given *world*.

**render** (world, camera)
	Renders *world* using *camera*.

**create_resource_package** (name) : ResourcePackage
	Returns the resource package with the given *package_name* name.

**destroy_resource_package** (package)
	Destroy a previously created resource *package*.
	Note that to unload the resources loaded by the package, you have to call
	ResourcePackage.unload() first.

**console_send** (table)
	Sends the given lua *table* to clients connected to the engine.
	Values can be either ``nil``, bool, number, string, table, array, Vector2, Vector3, Quaternion, Matrix4x4 or Color4.

**can_get** (type, name) : bool
	Returns whether the resource (type, name) is loaded.
	When resource autoload is enabled it always returns true.

**enable_resource_autoload** (enable)
	Sets whether resources should be automatically loaded when accessed.

**temp_count** () : int, int, int
	Returns the number of temporary objects used by Lua.

**set_temp_count** (nv, nq, nm)
	Sets the number of temporary objects used by Lua.

**guid** () : string
	Returns a new GUID.

DebugGui
========

**rect** (pos, size, material_resource, color)
	Draws a rectangle.

**image** (pos, size, material_resource, color)
	Draws an image.

**image_uv** (pos, size, uv0, uv1, material_resource, color)
	Draws an image with explicit UV coordinates.

**text** (pos, font_size, str, font_resource, material_resource, color)
	Draws text.

DebugLine
=========

**add_line** (debug_line, start, end, color)
	Adds a line from *start* to *end* with the given *color*.

**add_axes** (debug_line, tm, length)
	Adds lines for each axis with the given *length*.

**add_circle** (debug_line, center, radius, normal, color, [segments = 36])
	Adds a circle at *center* with the given *radius* and *normal* vector.

**add_cone** (debug_line, from, to, radius, color, [segments = 36])
	Adds a cone with the base centered at *from* and the tip at *to*.

**add_sphere** (debug_line, center, radius, color, [segments = 36])
	Adds a sphere at *center* with the given *radius*.

**add_obb** (debug_line, tm, half_extents, color)
	Adds an orientd bounding box. *tm* describes the position and orientation of
	the box. *half_extents* describes the size of the box along the axis.

**add_frustum** (debug_line, mvp, color)
	Adds a frustum defined by *mvp*.

**add_unit** (debug_line, tm, name, color)
	Adds the meshes from the unit *name*.

**reset** (debug_line)
	Resets all the lines.

**submit** (debug_line)
	Submits the lines to renderer for drawing.

Input
=====

Keyboard
--------

**name** () : string
	Returns the name of keyboard.

**connected** () : bool
	Returns whether the keyboard is connected and functioning.

**num_buttons** () : int
	Returns the number of buttons of the keyboard.

**num_axes** () : int
	Returns the number of axes of the keyboard.

**pressed** (id) : bool
	Returns whether the button *id* is pressed in the current frame.

**released** (id) : bool
	Returns whether the button *id* is released in the current frame.

**any_pressed** () : bool
	Returns whether any button is pressed in the current frame.

**any_released** () : bool
	Returns whether any button is released in the current frame.

**button_name** (id) : string
	Returns the name of the button *id*.

**button_id** (name) : int
	Returns the *id* of the button *name* or ``nil`` if no matching button is found.

Keyboard Button Names
~~~~~~~~~~~~~~~~~~~~~

* ``tab``, ``enter``, ``escape``, ``space``, ``backspace``
* ``num_lock``, ``numpad_enter``, ``numpad_.``, ``numpad_*``, ``numpad_+``, ``numpad_-``, ``numpad_/``, ``numpad_0``, ``numpad_1``, ``numpad_2``, ``numpad_3``, ``numpad_4``, ``numpad_5``, ``numpad_6``, ``numpad_7``, ``numpad_8``, ``numpad_9``
* ``f1``, ``f2``, ``f3``, ``f4``, ``f5``, ``f6``, ``f7``, ``f8``, ``f9``, ``f10``, ``f11``, ``f12``
* ``home``, ``left``, ``up``, ``right``, ``down``, ``page_up``, ``page_down``, ``ins``, ``del``, ``end``
* ``ctrl_left``, ``ctrl_right``, ``shift_left``, ``shift_right``, ``caps_lock``, ``alt_left``, ``alt_right``, ``super_left``, ``super_right``
* ``0``, ``1``, ``2``, ``3``, ``4``, ``5``, ``6``, ``7``, ``8``, ``9``
* ``a``, ``b``, ``c``, ``d``, ``e``, ``f``, ``g``, ``h``, ``i``, ``j``, ``k``, ``l``, ``m``, ``n``, ``o``, ``p``, ``q``, ``r``, ``s``, ``t``, ``u``, ``v``, ``w``, ``x``, ``y``, ``z``

Keyboard Axis Names
~~~~~~~~~~~~~~~~~~~

None.

Mouse
-----

**name** () : string
	Returns the name of the mouse.

**connected** () : bool
	Returns whether the mouse is connected and functioning.

**num_buttons** () : int
	Returns the number of buttons of the mouse.

**num_axes** () : int
	Returns the number of axes of the mouse.

**pressed** (id) : bool
	Returns whether the button *id* is pressed in the current frame.

**released** (id) : bool
	Returns whether the button *id* is released in the current frame.

**any_pressed** () : bool
	Returns whether any button is pressed in the current frame.

**any_released** () : bool
	Returns whether any button is released in the current frame.

**axis** (id) : Vector3
	Returns the value of the axis *id*.

**button_name** (id) : string
	Returns the name of the button *id*.

**axis_name** (id) : string
	Returns the name of the axis *id*.

**button_id** (name) : int
	Returns the *id* of the button *name* or ``nil`` if no matching button is found.

**axis_id** (name) : int
	Returns the *id* of the axis *name* or ``nil`` if no matching axis is found.

Mouse Button Names
~~~~~~~~~~~~~~~~~~

``left``, ``middle``, ``right``, ``extra_1``, ``extra_2``

Mouse Axis Names
~~~~~~~~~~~~~~~~

* ``cursor``: Returns the cursor position (x, y) in screen coordinates.
* ``cursor_delta``: Returns the delta of the cursor position (x, y) since last frame.
* ``wheel``: Returns the movement of the mouse wheel in the y axis. Positive values of y mean upward scrolling, negative values mean downward scrolling.

Touch
-----

**name** () : string
	Returns the name of the touch.

**connected** () : bool
	Returns whether the touch is connected and functioning.

**num_buttons** () : int
	Returns the number of buttons of the touch.

**num_axes** () : int
	Returns the number of axes of the touch.

**pressed** (id) : bool
	Returns whether the button *id* is pressed in the current frame.

**released** (id) : bool
	Returns whether the button *id* is released in the current frame.

**any_pressed** () : bool
	Returns whether any button is pressed in the current frame.

**any_released** () : bool
	Returns whether any button is released in the current frame.

**axis** (id) : Vector3
	Returns the value of the axis *id*.

**button_name** (id) : string
	Returns the name of the button *id*.

**axis_name** (id) : string
	Returns the name of the axis *id*.

**button_id** (name) : int
	Returns the *id* of the button *name* or ``nil`` if no matching button is found.

**axis_id** (name) : int
	Returns the *id* of the axis *name* or ``nil`` if no matching axis is found.

Pad1, Pad2, Pad3, Pad4
----------------------

**name** () : string
	Returns the name of the pad.

**connected** () : bool
	Returns whether the pad is connected and functioning.

**num_buttons** () : int
	Returns the number of buttons of the pad.

**num_axes** () : int
	Returns the number of axes of the pad.

**pressed** (id) : bool
	Returns whether the button *id* is pressed in the current frame.

**released** (id) : bool
	Returns whether the button *id* is released in the current frame.

**any_pressed** () : bool
	Returns whether any button is pressed in the current frame.

**any_released** () : bool
	Returns whether any button is released in the current frame.

**axis** (id) : Vector3
	Returns the value of the axis *id*.

**button_name** (id) : string
	Returns the name of the button *id*.

**axis_name** (id) : string
	Returns the name of the axis *id*.

**button_id** (name) : int
	Returns the *id* of the button *name* or ``nil`` if no matching button is found.

**axis_id** (name) : int
	Returns the *id* of the axis *name* or ``nil`` if no matching axis is found.

Pad Button Names
~~~~~~~~~~~~~~~~

* ``up``, ``down``, ``left``, ``right``
* ``start``, ``back``, ``guide``
* ``thumb_left``, ``thumb_right``
* ``shoulder_left``, ``shoulder_right``
* ``a``, ``b``, ``x``, ``y``

Pad Axis Names
~~~~~~~~~~~~~~

* ``left``, ``right``: Returns the direction (x, y) of the left or right thumbstick [-1; +1]. The z element represents the left or right trigger [0; +1].

Profiler
========

**enter_scope** (name)
	Starts a new profile scope with the given *name*.

**leave_scope** ()
	Ends the last profile scope.

**record** (name, value)
	Records *value* with the given *name*. Value can be either number or Vector3.

Display
=======

**modes** () : table
	Returns an array of display modes. See `Display Mode`_.

**set_mode** (id)
	Sets the display mode *id*.
	The initial display mode is automatically reset when the program terminates.

Display Mode
------------

Display mode is a lua table with 3 fields:

* ``id``: The id of the display mode.
* ``width``: The width of the display mode.
* ``height``: The height of the display mode.

Window
======

**show** ()
	Shows the window.

**hide** ()
	Hides the window.

**resize** (width, height)
	Resizes the window to *width* and *height*.

**move** (x, y)
	Moves the window to *x* and *y*.

**minimize** ()
	Minimizes the window.

**restore** ()
	Restores the window.

**title** () : string
	Returns the title of the window.

**set_title** (title)
	Sets the title of the window.

**show_cursor** (show)
	Sets whether to *show* the cursor.
