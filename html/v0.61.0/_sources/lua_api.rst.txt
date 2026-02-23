=================
Lua API reference
=================

AnimationStateMachine
=====================

**create** (asm, unit, state_machine_resource) : Id
	Creates a new state machine instance for *unit* and returns its id.

**destroy** (asm, state_machine)
	Destroys the *state_machine* instance.

**instance** (asm, unit) : Id
	Returns the ID of the state machine owned by the *unit*, or ``nil``.

**variable_id** (asm, state_machine, name) : Id
	Returns the ID of the variable *name* in the *state_machine*, or ``nil`` if
	the variable does not exist.

**variable** (asm, state_machine, variable_id) : number
	Returns the value of the *variable_id* in the *state_machine*.

**set_variable** (asm, state_machine, variable_id, value)
	Sets the *value* of the *variable_id* in the *state_machine*.

**trigger** (asm, state_machine, event)
	Triggers the *event* in the *state_machine*.

**set_state_machine** (asm, state_machine, state_machine_resource)
	Sets the *state_machine_resource* of *state_machine*.

DebugLine
=========

**add_line** (debug_line, start, end, color)
	Adds a line from *start* to *end* with the given *color*.

**add_axes** (debug_line, tm, length)
	Adds lines for each axis with the given *length*.

**add_arc** (debug_line, center, radius, plane_normal, midpoint_normal, color, [circle_segments = 36]);
	Adds an arc at *center* with the given *radius* and *plane_normal* and *midpoint_normal* vectors.

**add_circle** (debug_line, center, radius, normal, color, [segments = 36])
	Adds a circle at *center* with the given *radius* and *normal* vector.

**add_cone** (debug_line, from, to, radius, color, [segments = 36, rays = 36])
	Adds a cone with the base centered at *from* and the tip at *to*.

**add_sphere** (debug_line, center, radius, color, [segments = 36])
	Adds a sphere at *center* with the given *radius*.

**add_obb** (debug_line, tm, half_extents, color)
	Adds an orientd bounding box. *tm* describes the position and orientation of
	the box. *half_extents* describes the size of the box along the axis.

**add_frustum** (debug_line, mvp, color)
	Adds a frustum defined by *mvp*.

**reset** (debug_line)
	Resets all the lines.

**submit** (debug_line)
	Submits the lines to renderer for drawing.

Device
======

**argv** () : table
	Returns a table containing the command line parameters the engine was started with.

**platform** () : string
	Returns a string identifying what platform the engine is running on.
	It can be either ``android``, ``html5``, ``linux`` or ``windows``

**architecture** () : string
	Returns a string identifying what architecture the engine is running on.
	It can be either ``32-bit`` or ``64-bit``.

**version** () : string
	Returns a string identifying the engine version.
	The form is "major.minor.micro".

**build** () : string
	Returns a string identifying the engine build.
	It can be either ``debug``, ``development`` or ``release``.

**quit** ([exit_code])
	Quits the application. On platforms that support it, *exit_code* is
	returned to the system after the application exits.

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

	.. note::
		To unload the resources loaded by the package, you have to call
		ResourcePackage.unload() first.

**screenshot** (path)
	Captures a screenshot of the main window's backbuffer and saves it at *path* in PNG format.
	The global callback ``screenshot (path)`` will be called after the file is written to disk.

	.. note::
		Only available in debug and development builds.

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

**set_timestep_policy** (policy)
	Sets the timestep policy:

	* ``variable``: the timestep is the time it took for the previous frame to simulate. This is the default;
	* ``smoothed``: the timestep is computed as an average of the previous delta times.

**set_timestep_smoothing** (num_samples, num_outliers, average_cap)
	Sets the number of samples to be averaged, the outliers to be excluded and the maximum
	allowed deviation in percent from the previous average.  Given a delta time series DTs, and
	a previous average AVGp:

	* DTs = [ .33; .30; .32; .33; .24; .33; .25; .35; .33; .42 ]
	* AVGp = 0.33

	A call to set_smoothing(10, 2, 0.1) would compute the new average like so:

	1. Remove the 2 minimum and maximum values in DTs;
	2. Compute the average for the remaining 6 values in DTs: AVG = 0.323.
	3. Smooth the newly computed average: AVG = lerp(AVGp, AVG, 0.1) = 0.329.

Display
=======

**modes** () : table
	Returns an array of `DisplayMode`_ tables.

**set_mode** (id)
	Sets the display mode *id*.
	The initial display mode is automatically reset when the program terminates.

DisplayMode
-----------

DisplayMode is a lua table with 3 fields:

* ``id``: The id of the display mode.
* ``width``: The width of the display mode.
* ``height``: The height of the display mode.

Gui
===

**move** (gui, pos)
	Moves the Gui to *pos*.

**triangle** (gui, a, b, c [, color, depth])
	Draws a triangle defined by vertices *a*, *b* and *c*.
	Objects with a lower depth are drawn in front. If depth is omitted its value
	is set to 0.

**triangle_3d** (gui, local_pose, a, b, c [, color, depth])
	Draws a 3D triangle defined by vertices *a*, *b* and *c*.

**rect** (gui, pos, size [, color])
	Draws a rectangle.
	If pos is a Vector3, the z element specifies the drawing depth. Objects with
	a lower depth are drawn in front. If pos is a Vector2, the z elements is set
	to 0.

**rect_3d** (gui, local_pose, pos, size [, color, depth])
	Draws a 3D rectangle.

**image** (gui, pos, size, material_resource [, color])
	Draws an image.
	If pos is a Vector3, the z element specifies the drawing depth. Objects with
	a lower depth are drawn in front. If pos is a Vector2, the z elements is set
	to 0.

**image_3d** (gui, local_pose, pos, size, material_resource [, color, depth])
	Draws a 3D image.

**image_uv** (gui, pos, size, uv0, uv1, material_resource [, color])
	Draws an image with explicit UV coordinates.
	If pos is a Vector3, the z element specifies the drawing depth. Objects with
	a lower depth are drawn in front. If pos is a Vector2, the z elements is set
	to 0.

**image_3d_uv** (gui, local_pose, pos, size, uv0, uv1, material_resource [, color, depth])
	Draws a 3D image with explicit UV coordinates.

**text** (gui, pos, font_size, str, font_resource [, material_resource , color])
	Draws text.
	If pos is a Vector3, the z element specifies the drawing depth. Objects with
	a lower depth are drawn in front. If pos is a Vector2, the z elements is set
	to 0.

**text_extents** (gui, font_size, str, font_resource) : Vector2
	Returns the extents of the text *str* if drawn using *font_resource* at size
	*font_size*.

**text_3d** (gui, local_pose, pos, font_size, str, font_resource [, material_resource, color, depth])
	Draws 3D text.

**material** (material_resource) : Material
	Returns the material *material_resource*.

Input
=====

**events** () : table
	Returns an array of `InputEvent`_ tables.

InputEvent
----------

InputEvent is a lua table with 4 fields:

* ``id``: The id of the button or axis as returned by ``Keyboard.button_id()`` for example.
* ``type``: One of the values from `InputEventType`_.
* ``value``: The value of the axis as a Vector3 or ``nil`` when ``type`` is not an axis event.
* ``device``: The input device that generated the event. This is one of ``Keyboard``, ``Mouse``, ``Pad1`` etc.

InputEventType
--------------

* ``BUTTON_PRESSED``: A button has been pressed.
* ``BUTTON_RELEASED``: A button has been released.
* ``AXIS_CHANGED``: An axis changed its value.

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
	Returns the *id* of the first button that was pressed in the current frame
	or ``nil`` if no buttons were pressed at all.

**any_released** () : bool
	Returns the *id* of the first button that was released in the current frame
	or ``nil`` if no buttons were released at all.

**button** (id) : float
	Returns the value of the button *id* in the range [0..1].

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
	Returns the *id* of the first button that was pressed in the current frame
	or ``nil`` if no buttons were pressed at all.

**any_released** () : bool
	Returns the *id* of the first button that was released in the current frame
	or ``nil`` if no buttons were released at all.

**button** (id) : float
	Returns the value of the button *id* in the range [0..1].

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
	Returns the *id* of the first button that was pressed in the current frame
	or ``nil`` if no buttons were pressed at all.

**any_released** () : bool
	Returns the *id* of the first button that was released in the current frame
	or ``nil`` if no buttons were released at all.

**button** (id) : float
	Returns the value of the button *id* in the range [0..1].

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
	Returns the *id* of the first button that was pressed in the current frame
	or ``nil`` if no buttons were pressed at all.

**any_released** () : bool
	Returns the *id* of the first button that was released in the current frame
	or ``nil`` if no buttons were released at all.

**button** (id) : float
	Returns the value of the button *id* in the range [0..1].

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

**deadzone** (id) : deadzone_mode, deadzone_size
	Returns the deadzone mode and size for the axis *id*.

**set_deadzone** (id, deadzone_mode, deadzone_size)
	Sets the *deadzone_mode* and *deadzone_size* for the axis *id*.

Pad Button Names
~~~~~~~~~~~~~~~~

* ``up``, ``down``, ``left``, ``right``
* ``start``, ``back``, ``guide``
* ``thumb_left``, ``thumb_right``
* ``shoulder_left``, ``shoulder_right``
* ``a``, ``b``, ``x``, ``y``

Pad Axis Names
~~~~~~~~~~~~~~

* ``left``, ``right``: Returns the direction (x, y) of the left or right thumbstick [-1; +1].
* ``trigger_left``, ``trigger_right``: The z element represents the left or right trigger [0; +1].

Material
========

**set_float** (material, name, value)
	Sets the *value* of the variable *name*.

**set_vector2** (material, name, value)
	Sets the *value* of the variable *name*.

**set_vector3** (material, name, value)
	Sets the *value* of the variable *name*.

**set_vector4** (material, name, value)
	Sets the *value* of the variable *name*.

**set_matrix4x4** (material, name, value)
	Sets the *value* of the variable *name*.

**set_texture** (material, sampler_name, texture_name)
	Sets the *texture_resource* of the sampler *sampler_name*.

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
	Returns the length of *a*.

**length_squared** (a) : float
	Returns the squared length of *a*.

**set_length** (a, len)
	Sets the length of *a* to *len*.

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

**equal** (a, b) : bool
	Returns true whether the matrices *a* and *b* are equal.

**transpose** (m) : Matrix4x4
	Transposes the matrix *m* and returns the result.

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

**translation** (m) : Vector3
	Returns the translation portion of the matrix *m*.

**set_translation** (m, t)
	Sets the translation portion of the matrix *m*.

**rotation** (m) : Quaternion
	Returns the rotation portion of the matrix *m*.

**set_rotation** (m, r)
	Sets the rotation portion of the matrix *m*.

**scale** (m) : Vector3
	Returns the scale of the matrix *m*.

**set_scale** (m, r)
	Sets the scale of the matrix *m*.

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

**obb_intersects_frustum** (obb_tm, obb_half_extents, n0, d0, n1, d1, n2, d2, n3, d3, n4, d4, n4, d5) : bool
	Returns whether the oriented bounding box (obb_tm, obb_half_extents) intersects
	the frustum defined by the planes ((n0, d0), (n1, d1), ..., (n5, d5)). The
	planes be given in BRTLNF order, where B = bottom plane, R = right plane etc.

**obb_vertices** (obb_tm, obb_half_extents) : v0, v1, v2, v3, v4, v5, v6, v7
	Returns the vertices of the oriented bounding box (obb_tm, obb_half_extents).

**obb_merge** (a_tm, a_half_extents, b_tm, b_half_extents, ...) : Matrix4x4, Vector3
	Returns a new box which encloses both the box A (a_tm, a_half_extents) and B
	(b_tm, b_half_extents) and any number of boxes after B.
	Note that the returned OBB will have the same orientation as A and won't
	necessarily be the smallest enclosing box.

PhysicsWorld
=============

**gravity** (pw) : Vector3
	Returns the gravity.

**set_gravity** (pw, gravity)
	Sets the gravity.

**cast_ray** (pw, from, dir, length) : hit, collision_pos, normal, time, UnitId, Actor
	Casts a ray into the physics world and returns the closest actor it intersects with.
	If *hit* is true the following return values contain the *collision_pos* in
	world space, the *normal* of the surface that was hit, the time of impact
	in [0..1] and the *unit* and the *actor* that was hit.

**cast_ray_all** (pw, from, dir, length) : table
	Casts a ray into the physics world and returns all the
	actors it intersects with as an array of `RaycastHit`_ tables.

**cast_sphere** (pw, from, radius, dir, length) : hit, collision_pos, normal, time, UnitId, Actor
	Casts a sphere into the physics world and returns the closest actor it intersects with.
	If *hit* is true the following return values contain the *collision_pos* in
	world space, the *normal* of the surface that was hit, the time of impact
	in [0..1] and the *unit* and the *actor* that was hit.

**cast_box** (pw, from, half_extents, dir, length) : hit, collision_pos, normal, time, UnitId, Actor
	Casts a box into the physics world and returns the closest actor it intersects with.
	If *hit* is true the following return values contain the *collision_pos* in
	world space, the *normal* of the surface that was hit, the time of impact
	in [0..1] and the *unit* and the *actor* that was hit.

**enable_debug_drawing** (pw, enable)
	Sets whether to *enable* debug drawing.

RaycastHit
----------

RaycastHit is a lua table with 5 fields:

* ``[1]``: The collision position in world space.
* ``[2]``: The normal of the surface that was hit.
* ``[3]``: The time of impact in [0..1].
* ``[4]``: The unit that was hit.
* ``[5]``: The actor that was hit.

Actor
-----

**actor_destroy** (pw, actor)
	Destroys the *actor* instance.

**actor_instance** (pw, unit) : Id
	Returns the ID of the actor owned by the *unit*, or ``nil``.

**actor_world_position** (pw, actor) : Vector3
	Returns the world position of the *actor*.

**actor_world_rotation** (pw, actor) : Quaternion
	Returns the world rotation of the *actor*.

**actor_world_pose** (pw, actor) : Matrix4x4
	Returns the world pose of the *actor*.

**actor_teleport_world_position** (pw, actor, position)
	Teleports the *actor* to the given world *position*.

**actor_teleport_world_rotation** (pw, actor, rotation)
	Teleports the *actor* to the given world *rotation*.

**actor_teleport_world_pose** (pw, actor, pose)
	Teleports the *actor* to the given world *pose*.

**actor_center_of_mass** (pw, actor) : Vector3
	Returns the center of mass of the *actor*.

**actor_enable_gravity** (pw, actor)
	Enables gravity for the *actor*.

**actor_disable_gravity** (pw, actor)
	Disables gravity for the *actor*.

**actor_enable_collision** (pw, actor)
	Enables collision detection for the *actor*.

**actor_disable_collision** (pw, actor)
	Disables collision detection for the *actor*.

**actor_set_collision_filter** (pw, actor, name)
	Sets the collision filter of the *actor*.

**actor_set_kinematic** (pw, actor, kinematic)
	Sets whether the *actor* is *kinematic* or not.

	.. note::
		This call has no effect on static actors.

**actor_is_static** (pw, actor) : bool
	Returns whether the *actor* is static.

**actor_is_dynamic** (pw, actor) bool
	Returns whether the *actor* is dynamic.

**actor_is_kinematic** (pw, actor) : bool
	Returns whether the *actor* is kinematic (keyframed).

**actor_is_nonkinematic** (pw, actor) : bool
	Returns whether the *actor* is nonkinematic (i.e. dynamic and not kinematic).

**actor_linear_damping** (pw, actor) : float
	Returns the linear damping of the *actor*.

**actor_set_linear_damping** (pw, actor, damping)
	Sets the linear *damping* of the *actor*.

**actor_angular_damping** (pw, actor) : float
	Returns the angular damping rate of the *actor*.

**actor_set_angular_damping** (pw, actor, rate)
	Sets the angular damping *rate* of the *actor*.

**actor_linear_velocity** (pw, actor) : Vector3
	Returns the linear velocity of the *actor*.

**actor_set_linear_velocity** (pw, actor, velocity)
	Sets the linear *velocity* of the *actor*.

	.. note::
		This call only affects nonkinematic actors.

**actor_angular_velocity** (pw, actor) : Vector3
	Returns the angular velocity of the *actor*.

**actor_set_angular_velocity** (pw, actor, velocity)
	Sets the angular *velocity* of the *actor*.

	.. note::
		This call only affects nonkinematic actors.

**actor_add_impulse** (pw, actor, impulse)
	Adds a linear *impulse* (acting along the center of mass) to the *actor*.

	.. note::
		This call only affects nonkinematic actors.

**actor_add_impulse_at** (pw, actor, impulse, position)
	Adds a linear *impulse* (acting along the world position *pos*) to the *actor*.

	.. note::
		This call only affects nonkinematic actors.

**actor_add_torque_impulse** (pw, actor, impulse)
	Adds a torque *impulse* to the *actor*.

**actor_push** (pw, actor, velocity, mass)
	Pushes the *actor* as if it was hit by a point object with the given *mass*
	travelling at the given *velocity*.

	.. note::
		This call only affects nonkinematic actors.

**actor_push_at** (pw, actor, velocity, mass, position)
	Like push() but applies the force at the world *position*.

	.. note::
		This call only affects nonkinematic actors.

**actor_is_sleeping** (pw, actor) : bool
	Returns whether the *actor* is sleeping.

**actor_wake_up** (pw, actor)
	Wakes the *actor* up.

**actor_debug_draw** (pw, actor, debug_line [, color])
	Adds the *actor*'s debug geometry to *lines*.

Mover
-----

**mover_create** (pw, unit, height, radius, max_slope_angle, collision_filter) : Id
	Creates a new mover instance for the *unit*.

**mover_destroy** (pw, mover)
	Destroys the *mover*.

**mover** (pw, unit) : Id
	Returns the ID of the mover owned by the *unit*.

**mover_set_height** (pw, mover, height)
	Sets the *height* of the *mover* capsule.

**mover_radius** (pw, mover) : number
	Returns the radius of the *mover* capsule.

**mover_set_radius** (pw, mover, radius)
	Sets the *radius* of the *mover* capsule.

**mover_max_slope_angle** (pw, mover) : number
	Returns the max slope angle of the *mover*.

**mover_set_max_slope_angle** (pw, mover, angle)
	Sets the max slope *angle* of the *mover*.

**mover_set_collision_filter** (pw, mover, filter)
	Sets the collision *filter* of the *mover*.

**mover_position** (pw, mover) : Vector3
	Returns the position of the *mover*.

**mover_set_position** (pw, mover, position)
	Teleports the *mover* to the specified *position*.

**mover_center** (pw, mover) : Vector3
	Returns the center of the *mover* relative to the transform's position.

**mover_set_center** (pw, mover, center)
	Sets the center of the *mover* relative to the transform's position.

**mover_move** (pw, mover, delta)
	Attempts to move the *mover* by the specified *delta* vector.
	The *mover* will slide against physical actors.

**mover_collides_sides** (pw, mover) : bool
	Returns whether the *mover* collides sideways.

**mover_collides_up** (pw, mover) : bool
	Returns whether the *mover* collides upwards.

**mover_collides_down** (pw, mover) : bool
	Returns whether the *mover* collides downwards.

**mover_debug_draw** (pw, mover, debug_line [, color])
	Adds the *mover*'s debug geometry to *lines*.

Profiler
========

**enter_scope** (name)
	Starts a new profile scope with the given *name*.

**leave_scope** ()
	Ends the last profile scope.

**record** (name, value)
	Records *value* with the given *name*. Value can be either number or Vector3.

RenderWorld
===========

**enable_debug_drawing** (rw, enable)
	Sets whether to *enable* debug drawing.

Mesh
----

**mesh_create** (rw, unit, mesh_resource, geometry_name, material_resource, visible) : Id
	Creates a new mesh instance for *unit* and returns its id.

**mesh_destroy** (rw, mesh)
	Destroys the *mesh* instance.

**mesh_instance** (rw, unit) : Id
	Returns the ID of the mesh owned by the *unit*, or ``nil``.

**mesh_material** (rw, mesh) : Material
	Returns the material of the *mesh*.

**mesh_set_material** (rw, mesh, material)
	Sets the *material* of the *mesh*.

**mesh_set_visible** (rw, mesh, visible)
	Sets whether the *mesh* is *visible*.

**mesh_set_cast_shadows** (rw, mesh, cast_shadows)
	Sets whether the *mesh* cast shadows.

**mesh_obb** (rw, mesh) : Matrix4x4, Vector3
	Returns the Oriented-Bounding-Box of the *mesh* as (pose, half_extents).

**mesh_cast_ray** (rw, mesh, from, dir) : float
	Returns the distance along ray (from, dir) to intersection point with the *mesh* or -1.0 if no intersection.

Sprite
------

**sprite_create** (rw, unit, sprite_resource, material_resource, visible) : Id
	Creates a new sprite instance for the *unit* and returns its id.

**sprite_destroy** (rw, sprite)
	Destroys the *sprite* instance.

**sprite_instance** (rw, unit) : Id
	Returns the ID of the sprite owned by the *unit*, or ``nil``.

**sprite_material** (rw, sprite) : Material
	Returns the material of the *sprite*.

**sprite_set_material** (rw, sprite, material)
	Sets the *material* of the *sprite*.

**sprite_set_frame** (rw, sprite, index)
	Sets the frame *index* of the *sprite*.
	The *index* automatically wraps if it greater than the total number of
	frames in the sprite.

**sprite_set_visible** (rw, sprite, visible)
	Sets whether the *sprite* is *visible*.

**sprite_flip_x** (rw, sprite, flip)
	Sets whether to flip the *sprite* on the x-axis.

**sprite_flip_y** (rw, sprite, flip)
	Sets whether to flip the *sprite* on the y-axis.

**sprite_set_layer** (rw, sprite, layer)
	Sets the rendering *layer* of the *sprite*.

**sprite_set_depth** (rw, sprite, depth)
	Sets the rendering *depth* of the *sprite*.

**sprite_obb** (rw, sprite) : Matrix4x4, Vector3
	Returns the Oriented-Bounding-Box of the *sprite* as (pose, half_extents).

**sprite_cast_ray** (rw, sprite, from, dir) : float, int, int
	Returns (t, layer, depth), where *t* is the distance along ray (from, dir) to
	intersection point with the *sprite* or -1.0 if no intersection.

Light
-----

**light_create** (rw, unit, type, range, intensity, spot_angle, color) : Id
	Creates a new light for the *unit* and returns its id.
	Type can be either ``directional``, ``omni`` or ``spot``.

**light_destroy** (rw, light)
	Destroys the *light* instance.

**light_instance** (rw, unit) : Id
	Returns the ID of the light owned by the *unit*, or ``nil``.

**light_type** (rw, light) : string
	Returns the type of the *light*.
	It can be either ``directional``, ``omni`` or ``spot``.

**light_color** (rw, light) : Color4
	Returns the color of the *light*.

**light_range** (rw, light) : float
	Returns the range of the *light*.

**light_intensity** (rw, light) : float
	Returns the intensity of the *light*.

**light_spot_angle** (rw, light) : float
	Returns the spot angle of the *light*.

**light_shadow_bias** (rw, light) : float
	Returns the shadow bias of the *light*.

**light_set_type** (rw, light, type)
	Sets the *type* of the *light*.

**light_set_color** (rw, light, color)
	Sets the *color* of the *light*.

**light_set_range** (rw, light, range)
	Sets the *range* of the *light*.

**light_set_intensity** (rw, light, intensity)
	Sets the *intensity* of the *light*.

**light_set_spot_angle** (rw, light, angle)
	Sets the spot *angle* of the *light*.

**light_set_shadow_bias** (rw, light, bias)
	Sets the shadow *bias* of the *light*.

**light_set_cast_shadows** (rw, light, cast_shadows)
	Sets whether the *light* casts shadows.

**light_debug_draw** (rw, light, debug_line)
	Fills *debug_line* with debug lines from the *light*.

Fog
---

**fog_create** (rw, unit) : Id
	Creates a new fog instance for the *unit* and returns its id.
	Note that the fog component is limited to one instance per World. Every call
	to this function will overwrite any previously created fog instance.

**fog_destroy** (rw, fog)
	Destroys the *fog* instance.

**fog_instance** (rw, unit) : Id
	Returns the ID of the fog owned by the *unit*, or ``nil``.

**fog_set_color** (rw, fog, color)
	Sets the *color* of the *fog*.

**fog_set_density** (rw, fog, density)
	Sets the *density* of the *fog*.

**fog_set_range_min** (rw, fog, range)
	Sets the minimum *range* of the *fog*.

**fog_set_range_max** (rw, fog, range)
	Sets the maximum *range* of the *fog*.

**fog_set_sun_blend** (rw, fog, sun_blend)
	Sets the sun *blend* of the *fog*.

**fog_set_enabled** (rw, fog, enable)
	Sets whether the *fog* is *enabled*.

ResourcePackage
===============

**load** (package)
	Loads all the resources in the *package*.

	.. note::
		The resources are not immediately available after the call is made,
		instead, you have to poll for completion with has_loaded().

**unload** (package)
	Unloads all the resources in the *package*.

**flush** (package)
	Waits until the *package* has been loaded.

**has_loaded** (package) : bool
	Returns whether the *package* has been loaded.

SceneGraph
==========

**create** (sg, unit, position, rotation, scale) : Id
	Creates the transform for the *unit* and returns its ID.

**destroy** (sg, transform)
	Destroys the *transform* instance.

**instance** (sg, unit) : Id
	Returns the ID of the transform owned by the *unit*, or ``nil``.

**owner** (sg, transform) : UnitId
	Returns the unit that owns *transform*.

**local_position** (sg, transform) : Vector3
	Returns the local position of the *transform*.

**local_rotation** (sg, transform) : Quaternion
	Returns the local rotation of the *transform*.

**local_scale** (sg, transform) : Vector3
	Returns the local scale of the *transform*.

**local_pose** (sg, transform) : Matrix4x4
	Returns the local pose of the *transform*.

**world_position** (sg, transform) : Vector3
	Returns the world position of the *transform*.

**world_rotation** (sg, transform) : Quaternion
	Returns the world rotation of the *transform*.

**world_pose** (sg, transform) : Matrix4x4
	Returns the world pose of the *transform*.

**set_local_position** (sg, transform, position)
	Sets the local *position* of the *transform*.

**set_local_rotation** (sg, transform, rotation)
	Sets the local *rotation* of the *transform*.

**set_local_scale** (sg, transform, scale)
	Sets the local *scale* of the *transform*.

**set_local_pose** (sg, transform, pose)
	Sets the local *pose* of the *transform*.

**link** (sg, parent, child, child_local_position, child_local_rotation, child_local_scale)
	Links *child* to *parent*. After linking the child will follow its
	parent. Set child_local_* to modify the child position after it has been
	linked to the parent, otherwise che child will be positioned at the
	location of its parent.

**unlink** (sg, child)
	Unlinks *child* from its parent if it has any. After unlinking, the local
	pose of the *child* is set to its previous world pose.

**parent** (sg, child) : Id
	Returns the parent of the instance *child* or ``nil``
	if *child* has no parent.

**first_child** (sg, parent) : Id
	Returns the first child of the instance *parent* or ``nil``
	if *parent* has no children.

**next_sibling** (sg, child) : Id
	Returns the next sibling of the instance *child* or ``nil``
	if *child* has no sibling.

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

**set_group_volume** (sound_world, group, volume)
	Sets the *volume* of the sound *group*. The volume of the sounds within
	*group* is multiplied by the group's volume.

UnitManager
===========

**create** ([world]) : UnitId
	Creates a new empty unit. If *world* is specified, the unit will be owned by
	that world.

**destroy** (unit)
	Destroys the given *unit*.

**alive** (unit) : bool
	Returns whether the unit is alive.

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

**maximize** ()
	Maximizes the window.

**restore** ()
	Restores the window.

**title** () : string
	Returns the title of the window.

**set_title** (title)
	Sets the title of the window.

**show_cursor** (show)
	Sets whether to *show* the cursor.

**set_fullscreen** (fullscreen)
	Sets whether the window is *fullscreen*.

**set_cursor** (cursor)
	Sets the mouse *cursor* on this window. Cursor can be any of ``arrow``,
	``hand``, ``text_input``, ``corner_top_left``, ``corner_top_right``,
	``corner_bottom_left``, ``corner_bottom_right``, ``size_horizontal``,
	``size_vertical`` or ``wait``.

**set_cursor_mode** (mode) : bool
    Sets the mouse cursor *mode* on this window and returns true if successful.
    Mode can be either ``normal`` or ``disabled``.  Setting the mode to
    ``disabled`` hides the cursor and automatically re-centers it every time it
    is moved. On some platforms or backends disabling the cursor is not allowed
    or only possible under specific circumnstances (e.g. when the cursor is
    inside the window rectangle).

World
=====

**spawn_unit** (world, name, [position, rotation, scale]) : UnitId
	Spawns a new instance of the unit *name* at the given *position*, *rotation* and *scale*.

**spawn_empty_unit** (world) : UnitId
	Spawns a new empty unit and returns its id.

**destroy_unit** (world, unit)
	Destroys the given *unit*.

**num_units** (world) : int
	Returns the number of units in the *world*.

**units** (world) : table
	Returns all the the units in the world in a table.

**unit_by_name** (world, name) : UnitId
	Returns the unit with the given Level Editor *name* or ``nil`` if no such unit is found.
	If there are multiple units with the same name, a random one will be returned.

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

**create_screen_gui** (world) : Gui
	Creates a new Gui for 2D drawing.

**create_world_gui** (world) : Gui
	Creates a new Gui for 3D drawing.

**destroy_gui** (world, gui)
	Destroys the *gui*.

**scene_graph** (world) : SceneGraph
	Returns the scene graph.

**render_world** (world) : RenderWorld
	Returns the render sub-world.

**physics_world** (world) : PhysicsWorld
	Returns the physics sub-world.

**sound_world** (world) : SoundWorld
	Returns the sound sub-world.

**animation_state_machine** (world) : AnimationStateMachine
	Returns the animation state machine.

Camera
------

**camera_create** (world, unit, projection, fov, far_range, near_range) : Id
	Creates a new camera for *unit* and returns its id.
	Projection can be either ``orthographic`` or ``perspective``.

**camera_destroy** (world, camera)
	Destroys the *camera* instance.

**camera_instance** (world, unit) : Id
	Returns the ID of the camera owned by the *unit*, or ``nil``.

**camera_set_projection_type** (world, camera, projection)
	Sets the projection type of the *camera*.
	Projection can be either ``orthographic`` or ``perspective``.

**camera_projection_type** (world, camera) : string
	Returns the projection type of the *camera*.
	It can be either ``orthographic`` or ``perspective``.

**camera_fov** (world, camera) : float
	Returns the field-of-view of the *camera* in degrees.

**camera_set_fov** (world, camera, fov)
	Sets the field-of-view of the *camera* in degrees.

**camera_near_clip_distance** (world, camera) : float
	Returns the near clip distance of the *camera*.

**camera_set_near_clip_distance** (world, camera, near)
	Sets the near clip distance of the *camera*.

**camera_far_clip_distance** (world, camera) : float
	Returns the far clip distance of the *camera*.

**camera_set_far_clip_distance** (world, camera, far)
	Sets the far clip distance of the *camera*.

**camera_set_orthographic_size** (world, camera, half_size)
	Sets the vertical *half_size* of the orthographic view volume.
	The horizontal size is proportional to the viewport's aspect ratio.

**camera_screen_to_world** (world, camera, pos) : Vector3
	Returns *pos* from screen-space to world-space coordinates.

**camera_world_to_screen** (world, camera, pos) : Vector3
	Returns *pos* from world-space to screen-space coordinates.

Sound
-----

**play_sound** (world, name, [loop = false, volume = 1.0, range = 70.0, position = nil, group = nil]) : SoundInstanceId
	Plays the sound with the given *name*. If the listener is further away than
	*range* the sound will not be heard. If *position* is not specified the
	sound will be played as a 2d-sound (i.e. no attenuation nor spatialization).
	*group* is a string that identifies the sound's group, see ``SoundWorld.set_group_volume()``.

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

**destroy_level** (world, level)
	Destroys a level previously loaded by load_level(). Only units directly
	spawned by the level are unspawned.
