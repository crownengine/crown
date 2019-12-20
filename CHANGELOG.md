Changelog
=========

0.37.0
------
*DD MMM YYYY*

* runtime: added RenderWorld.mesh_set_material()
* runtime: added the ability to scale the shape of colliders at Unit spawn time
* runtime: added World.unit_by_name()
* runtime: fixed an issue that caused PhysicsWorld.set_gravity() to re-enable gravity to actors that previously disabled it with PhysicsWorld.actor_disable_gravity()
* runtime: fixed an issue that prevented kinematic actors to be controlled via the SceneGraph
* runtime: fixed an issue that prevented PhysicsWorld.actor_center_of_mass() to be called for static actors
* runtime: fixed an issue that prevented PhysicsWorld.actor_world_{position,rotation,pose}() to be called for static actors
* runtime: fixed an issue where a regular Matrix4x4 was returned if Matrix4x4Box is called without arguments
* runtime: removed "io" and "os" libraries from Lua API
* runtime: small fixes and performance improvements
* runtime: sprite's frame number now wraps if it is greater than the total number of frames in the sprite
* tools: added the ability to specify a circle collider in the Sprite Importer
* tools: added the ability to specify the actor class in the Sprite Importer
* tools: fixed a crash when entering empty commands in the console
* tools: fixed an issue that caused the Level Editor to not correctly save a level specified from command line
* tools: fixed an issue that could cause the Level Editor to crash when large number of TCP/IP packets were sent to it
* tools: fixed an issue that could cause the Level Editor to incorreclty parse identifiers in SJSON files
* tools: fixed an issue that prevented some operations in the Level Editor from being (un/re)done
* tools: fixed an issue that prevented the data compiler from restoring and saving its state when launched by the Level Editor
* tools: the Data Compiler will now track data "requirements" and automatically include them in packages when it's needed
* tools: the game will now be started or stopped according to its running state when launched from the Level Editor
* tools: the Properties Panel now accepts more sensible numeric ranges
* tools: the Properties Panel now allows the user to modify most Unit's component properties
