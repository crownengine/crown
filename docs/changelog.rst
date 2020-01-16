Changelog
=========

0.37.0
------
*DD MMM YYYY*

**Runtime**

* added Material.set_vector4() and Material.set_matrix4x4()
* added PhysicsWorld.actor_destroy()
* added RenderWorld.mesh_material(), RenderWorld.mesh_set_material() and RenderWorld.sprite_material()
* added the ability to scale the shape of colliders at Unit spawn time
* added World.unit_by_name() to retrieve unit by its name in the Level Editor
* bumped minimum Android version to 7.0+
* bumped minimum OpenGL version to 3.2+ for Linux
* fixed an issue that caused PhysicsWorld.set_gravity() to re-enable gravity to actors that previously disabled it with PhysicsWorld.actor_disable_gravity()
* fixed an issue that prevented kinematic actors to be controlled via the SceneGraph
* fixed an issue that prevented PhysicsWorld.actor_center_of_mass() to be called for static actors
* fixed an issue that prevented PhysicsWorld.actor_world_{position,rotation,pose}() to be called for static actors
* fixed an issue that reset the sprite animation to the beginning even when loop was set to false
* fixed an issue where a regular Matrix4x4 was returned if Matrix4x4Box is called without arguments
* removed "io" and "os" libraries from Lua API
* small fixes and performance improvements
* sprite's frame number now wraps if it is greater than the total number of frames in the sprite

**Tools**

* added the ability to specify a circle collider in the Sprite Importer
* added the ability to specify the actor class in the Sprite Importer
* fixed a crash when entering empty commands in the console
* fixed an issue that caused the Level Editor to not correctly save a level specified from command line
* fixed an issue that could cause the Level Editor to crash when large number of TCP/IP packets were sent to it
* fixed an issue that could cause the Level Editor to incorreclty parse identifiers in SJSON files
* fixed an issue that generated wrong render states when blending is enabled while no blend funtion/equation is specified
* fixed an issue that prevented some operations in the Level Editor from being (un/re)done
* fixed an issue that prevented the data compiler from restoring and saving its state when launched by the Level Editor
* improved the numeric entry widget which now takes less space and provides more convenient input workflows
* the Data Compiler will now track data "requirements" and automatically include them in packages when it's needed
* the game will now be started or stopped according to its running state when launched from the Level Editor
* the Properties Panel now accepts more sensible numeric ranges
* the Properties Panel now allows the user to modify most Unit's component properties
