/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "array.h"
#include "device.h"
#include "lua_environment.h"
#include "lua_stack.h"
#include "resource_manager.h"
#include "scene_graph.h"
#include "sound_world.h"
#include "temp_allocator.h"
#include "world.h"

namespace crown
{

struct ProjectionInfo
{
	const char* name;
	ProjectionType::Enum type;
};

static ProjectionInfo s_projection[] =
{
	{ "orthographic", ProjectionType::ORTHOGRAPHIC },
	{ "perspective",  ProjectionType::PERSPECTIVE  }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_projection) == ProjectionType::COUNT);

static ProjectionType::Enum name_to_projection_type(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_projection); ++i)
	{
		if (strcmp(s_projection[i].name, name) == 0)
			return s_projection[i].type;
	}

	LUA_ASSERT(false, stack, "Unknown projection: %s", name);
	return ProjectionType::COUNT;
}

static int world_spawn_unit(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos = stack.num_args() > 2 ? stack.get_vector3(3) : VECTOR3_ZERO;
	const Quaternion& rot = stack.num_args() > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;

	LUA_ASSERT(device()->resource_manager()->can_get(UNIT_TYPE, name), stack, "Unit not found");

	UnitId unit = world->spawn_unit(name, pos, rot);
	stack.push_unit(unit);
	return 1;
}

static int world_destroy_unit(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_unit(stack.get_unit(2));
	return 0;
}

static int world_num_units(lua_State* L)
{
	LuaStack stack(L);
	stack.push_int(stack.get_world(1)->num_units());
	return 1;
}

static int world_units(lua_State* L)
{
	LuaStack stack(L);

	TempAllocator1024 alloc;
	Array<UnitId> units(alloc);
	stack.get_world(1)->units(units);

	const uint32_t num = array::size(units);

	stack.push_table(num);
	for (uint32_t i = 0; i < num; ++i)
	{
		stack.push_key_begin((int32_t) i + 1);
		stack.push_unit(units[i]);
		stack.push_key_end();
	}

	return 1;
}

static int world_camera(lua_State* L)
{
	LuaStack stack(L);
	stack.push_camera(stack.get_world(1)->camera(stack.get_unit(2)));
	return 1;
}

static int camera_set_projection_type(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_projection_type(stack.get_camera(2)
		, name_to_projection_type(stack, stack.get_string(3))
		);
	return 0;
}

static int camera_projection_type(lua_State* L)
{
	LuaStack stack(L);
	ProjectionType::Enum type = stack.get_world(1)->camera_projection_type(stack.get_camera(2));
	stack.push_string(s_projection[type].name);
	return 1;
}

static int camera_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_fov(stack.get_camera(2)));
	return 1;
}

static int camera_set_fov(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_fov(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_aspect(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_aspect(stack.get_camera(2)));
	return 1;
}

static int camera_set_aspect(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_aspect(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_near_clip_distance(stack.get_camera(2)));
	return 1;
}

static int camera_set_near_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_near_clip_distance(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_world(1)->camera_far_clip_distance(stack.get_camera(2)));
	return 1;
}

static int camera_set_far_clip_distance(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_far_clip_distance(stack.get_camera(2), stack.get_float(3));
	return 0;
}

static int camera_set_orthographic_metrics(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_orthographic_metrics(stack.get_camera(2), stack.get_float(3), stack.get_float(4),
		stack.get_float(5), stack.get_float(6));
	return 0;
}

static int camera_set_viewport_metrics(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_camera_viewport_metrics(stack.get_camera(2), stack.get_int(3), stack.get_int(4),
		stack.get_int(5), stack.get_int(6));
	return 0;
}

static int camera_screen_to_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_world(1)->camera_screen_to_world(stack.get_camera(2), stack.get_vector3(3)));
	return 1;
}

static int camera_world_to_screen(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_world(1)->camera_world_to_screen(stack.get_camera(2), stack.get_vector3(3)));
	return 1;
}

static int world_update_animations(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_animations(stack.get_float(2));
	return 0;
}

static int world_update_scene(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update_scene(stack.get_float(2));
	return 0;
}

static int world_update(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->update(stack.get_float(2));
	return 0;
}

static int world_play_sound(lua_State* L)
{
	LuaStack stack(L);
	World* world = stack.get_world(1);
	const StringId64 name = stack.get_resource_id(2);
	const int32_t nargs = stack.num_args();
	const bool loop = nargs > 2 ? stack.get_bool(3) : false;
	const float volume = nargs > 3 ? stack.get_float(4) : 1.0f;
	const Vector3& pos = nargs > 4 ? stack.get_vector3(5) : VECTOR3_ZERO;
	const float range = nargs > 5 ? stack.get_float(6) : 1000.0f;

	LUA_ASSERT(device()->resource_manager()->can_get(SOUND_TYPE, name), stack, "Sound not found");

	stack.push_sound_instance_id(world->play_sound(name, loop, volume, pos, range));
	return 1;
}

static int world_stop_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->stop_sound(stack.get_sound_instance_id(2));
	return 0;
}

static int world_link_sound(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->link_sound(stack.get_sound_instance_id(2),
		stack.get_unit(3),
		stack.get_int(4));
	return 0;
}

static int world_set_listener_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_listener_pose(stack.get_matrix4x4(2));
	return 0;
}

static int world_set_sound_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_position(stack.get_sound_instance_id(2),
		stack.get_vector3(3));
	return 0;
}

static int world_set_sound_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_range(stack.get_sound_instance_id(2),
		stack.get_float(3));
	return 0;
}

static int world_set_sound_volume(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->set_sound_volume(stack.get_sound_instance_id(2),
		stack.get_float(3));
	return 0;
}

static int world_create_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.push_debug_line(stack.get_world(1)->create_debug_line(stack.get_bool(2)));
	return 1;
}

static int world_destroy_debug_line(lua_State* L)
{
	LuaStack stack(L);
	stack.get_world(1)->destroy_debug_line(*stack.get_debug_line(2));
	return 0;
}

static int world_load_level(lua_State* L)
{
	LuaStack stack(L);
	const StringId64 name = stack.get_resource_id(2);
	const Vector3& pos = stack.num_args() > 2 ? stack.get_vector3(3) : VECTOR3_ZERO;
	const Quaternion& rot = stack.num_args() > 3 ? stack.get_quaternion(4) : QUATERNION_IDENTITY;
	LUA_ASSERT(device()->resource_manager()->can_get(LEVEL_TYPE, name), stack, "Level not found");
	stack.push_level(stack.get_world(1)->load_level(name, pos, rot));
	return 1;
}

static int world_scene_graph(lua_State* L)
{
	LuaStack stack(L);
	stack.push_scene_graph(stack.get_world(1)->scene_graph());
	return 1;
}

static int world_render_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_render_world(stack.get_world(1)->render_world());
	return 1;
}

static int world_physics_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_physics_world(stack.get_world(1)->physics_world());
	return 1;
}

static int world_sound_world(lua_State* L)
{
	LuaStack stack(L);
	stack.push_sound_world(stack.get_world(1)->sound_world());
	return 1;
}

static int world_tostring(lua_State* L)
{
	LuaStack stack(L);
	World* w = stack.get_world(1);
	stack.push_fstring("World (%p)", w);
	return 1;
}

static int scene_graph_create(lua_State* L)
{
	LuaStack stack(L);
	stack.push_transform(stack.get_scene_graph(1)->create(stack.get_unit(2), MATRIX4X4_IDENTITY));
	return 1;
}

static int scene_graph_destroy(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->destroy(stack.get_transform(2));
	return 0;
}

static int scene_graph_transform_instances(lua_State* L)
{
	LuaStack stack(L);
	stack.push_transform(stack.get_scene_graph(1)->get(stack.get_unit(2)));
	return 1;
}

static int scene_graph_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_scene_graph(1)->local_position(stack.get_transform(2)));
	return 1;
}

static int scene_graph_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_scene_graph(1)->local_rotation(stack.get_transform(2)));
	return 1;
}

static int scene_graph_local_scale(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_scene_graph(1)->local_scale(stack.get_transform(2)));
	return 1;
}

static int scene_graph_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_scene_graph(1)->local_pose(stack.get_transform(2)));
	return 1;
}

static int scene_graph_world_position(lua_State* L)
{
	LuaStack stack(L);
	stack.push_vector3(stack.get_scene_graph(1)->world_position(stack.get_transform(2)));
	return 1;
}

static int scene_graph_world_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.push_quaternion(stack.get_scene_graph(1)->world_rotation(stack.get_transform(2)));
	return 1;
}

static int scene_graph_world_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.push_matrix4x4(stack.get_scene_graph(1)->world_pose(stack.get_transform(2)));
	return 1;
}

static int scene_graph_set_local_position(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_position(stack.get_transform(2), stack.get_vector3(3));
	return 0;
}

static int scene_graph_set_local_rotation(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_rotation(stack.get_transform(2), stack.get_quaternion(3));
	return 0;
}

static int scene_graph_set_local_scale(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_scale(stack.get_transform(2), stack.get_vector3(3));
	return 0;
}

static int scene_graph_set_local_pose(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->set_local_pose(stack.get_transform(2), stack.get_matrix4x4(3));
	return 0;
}

static int scene_graph_link(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->link(stack.get_transform(2), stack.get_transform(3));
	return 0;
}

static int scene_graph_unlink(lua_State* L)
{
	LuaStack stack(L);
	stack.get_scene_graph(1)->unlink(stack.get_transform(2));
	return 0;
}

static int unit_manager_create(lua_State* L)
{
	LuaStack stack(L);

	if (stack.num_args() == 1)
		stack.push_unit(device()->unit_manager()->create(*stack.get_world(1)));
	else
		stack.push_unit(device()->unit_manager()->create());

	return 1;
}

static int unit_manager_alive(lua_State* L)
{
	LuaStack stack(L);
	stack.push_bool(device()->unit_manager()->alive(stack.get_unit(1)));
	return 1;
}

void load_world(LuaEnvironment& env)
{
	env.load_module_function("World", "spawn_unit",                      world_spawn_unit);
	env.load_module_function("World", "destroy_unit",                    world_destroy_unit);
	env.load_module_function("World", "num_units",                       world_num_units);
	env.load_module_function("World", "units",                           world_units);

	env.load_module_function("World", "camera",                          world_camera);
	env.load_module_function("World", "set_camera_projection_type",      camera_set_projection_type);
	env.load_module_function("World", "camera_projection_type",          camera_projection_type);
	env.load_module_function("World", "camera_fov",                      camera_fov);
	env.load_module_function("World", "set_camera_fov",                  camera_set_fov);
	env.load_module_function("World", "camera_aspect",                   camera_aspect);
	env.load_module_function("World", "set_camera_aspect",               camera_set_aspect);
	env.load_module_function("World", "camera_near_clip_distance",       camera_near_clip_distance);
	env.load_module_function("World", "set_camera_near_clip_distance",   camera_set_near_clip_distance);
	env.load_module_function("World", "camera_far_clip_distance",        camera_far_clip_distance);
	env.load_module_function("World", "set_camera_far_clip_distance",    camera_set_far_clip_distance);
	env.load_module_function("World", "set_camera_orthographic_metrics", camera_set_orthographic_metrics);
	env.load_module_function("World", "set_camera_viewport_metrics",     camera_set_viewport_metrics);
	env.load_module_function("World", "camera_screen_to_world",          camera_screen_to_world);
	env.load_module_function("World", "camera_world_to_screen",          camera_world_to_screen);

	env.load_module_function("World", "update_animations",               world_update_animations);
	env.load_module_function("World", "update_scene",                    world_update_scene);
	env.load_module_function("World", "update",                          world_update);
	env.load_module_function("World", "play_sound",                      world_play_sound);
	env.load_module_function("World", "stop_sound",                      world_stop_sound);
	env.load_module_function("World", "link_sound",                      world_link_sound);
	env.load_module_function("World", "set_listener_pose",               world_set_listener_pose);
	env.load_module_function("World", "set_sound_position",              world_set_sound_position);
	env.load_module_function("World", "set_sound_range",                 world_set_sound_range);
	env.load_module_function("World", "set_sound_volume",                world_set_sound_volume);
	env.load_module_function("World", "create_debug_line",               world_create_debug_line);
	env.load_module_function("World", "destroy_debug_line",              world_destroy_debug_line);
	env.load_module_function("World", "load_level",                      world_load_level);
	env.load_module_function("World", "scene_graph",                     world_scene_graph);
	env.load_module_function("World", "render_world",                    world_render_world);
	env.load_module_function("World", "physics_world",                   world_physics_world);
	env.load_module_function("World", "sound_world",                     world_sound_world);
	env.load_module_function("World", "__index",                         "World");
	env.load_module_function("World", "__tostring",                      world_tostring);

	env.load_module_function("SceneGraph", "create",              scene_graph_create);
	env.load_module_function("SceneGraph", "destroy",             scene_graph_destroy);
	env.load_module_function("SceneGraph", "transform_instances", scene_graph_transform_instances);
	env.load_module_function("SceneGraph", "local_position",      scene_graph_local_position);
	env.load_module_function("SceneGraph", "local_rotation",      scene_graph_local_rotation);
	env.load_module_function("SceneGraph", "local_scale",         scene_graph_local_scale);
	env.load_module_function("SceneGraph", "local_pose",          scene_graph_local_pose);
	env.load_module_function("SceneGraph", "world_position",      scene_graph_world_position);
	env.load_module_function("SceneGraph", "world_rotation",      scene_graph_world_rotation);
	env.load_module_function("SceneGraph", "world_pose",          scene_graph_world_pose);
	env.load_module_function("SceneGraph", "set_local_position",  scene_graph_set_local_position);
	env.load_module_function("SceneGraph", "set_local_rotation",  scene_graph_set_local_rotation);
	env.load_module_function("SceneGraph", "set_local_scale",     scene_graph_set_local_scale);
	env.load_module_function("SceneGraph", "set_local_pose",      scene_graph_set_local_pose);
	env.load_module_function("SceneGraph", "link",                scene_graph_link);
	env.load_module_function("SceneGraph", "unlink",              scene_graph_unlink);

	env.load_module_function("UnitManager", "create", unit_manager_create);
	env.load_module_function("UnitManager", "alive",  unit_manager_alive);
}

} // namespace crown
