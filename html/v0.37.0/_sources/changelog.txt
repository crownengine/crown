Changelog
=========

0.37.0
------
*26 Jun 2020*

**Runtime**

* Added Material.set_vector4() and Material.set_matrix4x4()
* Added PhysicsWorld.actor_destroy()
* Added RenderWorld.mesh_material(), RenderWorld.mesh_set_material() and RenderWorld.sprite_material()
* Added the ability to hot-reload Lua files
* Added the ability to scale the shape of colliders at Unit spawn time
* Added Window.set_cursor_mode()
* Added World.unit_by_name() to retrieve unit by its name in the Level Editor
* Bumped minimum Android version to 7.0+
* Bumped minimum OpenGL version to 3.2+ for Linux
* Fixed an issue that caused PhysicsWorld.set_gravity() to re-enable gravity to actors that previously disabled it with PhysicsWorld.actor_disable_gravity()
* Fixed an issue that prevented kinematic actors to be controlled via the SceneGraph
* Fixed an issue that prevented PhysicsWorld.actor_center_of_mass() to be called for static actors
* Fixed an issue that prevented PhysicsWorld.actor_world_{position,rotation,pose}() to be called for static actors
* Fixed an issue that reset the sprite animation to the beginning even when loop was set to false
* Fixed an issue where a regular Matrix4x4 was returned if Matrix4x4Box is called without arguments
* Removed "io" and "os" libraries from Lua API
* Small fixes and performance improvements
* Sprite's frame number now wraps if it is greater than the total number of frames in the sprite

**Tools**

* Added the ability to specify a circle collider in the Sprite Importer
* Added the ability to specify the actor class in the Sprite Importer
* Added the ability to specify the destination of the console commands between Game and Editor
* Fixed a crash when entering empty commands in the console
* Fixed an issue that caused the Level Editor to not correctly save a level specified from command line
* Fixed an issue that could cause the Level Editor to crash when large number of TCP/IP packets were sent to it
* Fixed an issue that could cause the Level Editor to crash when scrolling through the console history
* Fixed an issue that could cause the Level Editor to incorreclty parse identifiers in SJSON files
* Fixed an issue that generated wrong render states when blending is enabled while no blend funtion/equation is specified
* Fixed an issue that prevented some operations in the Level Editor from being (un/re)done
* Fixed an issue that prevented the data compiler from restoring and saving its state when launched by the Level Editor
* Improved the numeric entry widget which now takes less space and provides more convenient input workflows
* Resources autoload is disabled when testing levels from Level Editor
* The Data Compiler will now track data "requirements" and automatically include them in packages when it's needed
* The game will now be started or stopped according to its running state when launched from the Level Editor
* The Properties Panel now accepts more sensible numeric ranges
* The Properties Panel now allows the user to modify most Unit's component properties
