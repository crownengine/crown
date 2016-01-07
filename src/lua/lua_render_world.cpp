/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

#include "quaternion.h"
#include "render_world.h"
#include "lua_stack.h"
#include "lua_environment.h"

namespace crown
{

struct LightInfo
{
	const char* name;
	LightType::Enum type;
};

static LightInfo s_light[] =
{
	{ "directional", LightType::DIRECTIONAL },
	{ "omni",        LightType::OMNI        },
	{ "spot",        LightType::SPOT        }
};
CE_STATIC_ASSERT(CE_COUNTOF(s_light) == LightType::COUNT);

static LightType::Enum name_to_light_type(LuaStack& stack, const char* name)
{
	for (uint32_t i = 0; i < CE_COUNTOF(s_light); ++i)
	{
		if (strcmp(s_light[i].name, name) == 0)
			return s_light[i].type;
	}

	LUA_ASSERT(false, stack, "Unknown light type: %s", name);
	return LightType::COUNT;
}

static int render_world_create_mesh(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	MeshRendererDesc desc;
	desc.mesh_resource = stack.get_resource_id(3);
	desc.mesh_name = stack.get_string_id(4);
	desc.material_resource = stack.get_resource_id(5);
	desc.visible = stack.get_bool(6);

	stack.push_mesh_instance(rw->create_mesh(unit, desc, MATRIX4X4_IDENTITY));
	return 1;
}

static int render_world_destroy_mesh(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->destroy_mesh(stack.get_mesh_instance(2));
	return 0;
}

static int render_world_mesh_instances(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);
	MeshInstance inst = rw->first_mesh(unit);

	stack.push_table();
	for (uint32_t i = 0; rw->is_valid(inst); ++i, inst = rw->next_mesh(inst))
	{
		stack.push_key_begin(i+1);
		stack.push_mesh_instance(inst);
		stack.push_key_end();
	}

	return 1;
}

static int render_world_mesh_obb(lua_State* L)
{
	LuaStack stack(L);
	OBB obb = stack.get_render_world(1)->mesh_obb(stack.get_mesh_instance(2));
	stack.push_matrix4x4(obb.tm);
	stack.push_vector3(obb.half_extents);
	return 2;
}

static int render_world_set_mesh_visible(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_mesh_visible(stack.get_mesh_instance(2), stack.get_bool(3));
	return 0;
}

static int render_world_create_sprite(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);

	SpriteRendererDesc desc;
	StringId64 sprite_resource = stack.get_resource_id(3);
	StringId64 material_resource = stack.get_resource_id(4);
	bool visible = stack.get_bool(5);

	stack.push_sprite_instance(rw->create_sprite(unit, desc, MATRIX4X4_IDENTITY));
	return 1;
}

static int render_world_destroy_sprite(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->destroy_sprite(stack.get_sprite_instance(2));
	return 0;
}

static int render_world_sprite_instances(lua_State* L)
{
	LuaStack stack(L);
	RenderWorld* rw = stack.get_render_world(1);
	UnitId unit = stack.get_unit(2);
	SpriteInstance inst = rw->first_sprite(unit);

	stack.push_table();
	for (uint32_t i = 0; rw->is_valid(inst); ++i, inst = rw->next_sprite(inst))
	{
		stack.push_key_begin(i+1);
		stack.push_sprite_instance(inst);
		stack.push_key_end();
	}
}

static int render_world_set_sprite_visible(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_sprite_visible(stack.get_sprite_instance(2), stack.get_bool(3));
	return 0;
}

static int render_world_set_sprite_frame(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_sprite_frame(stack.get_sprite_instance(2), stack.get_int(3));
	return 0;
}

static int render_world_create_light(lua_State* L)
{
	LuaStack stack(L);
	LightDesc ld;
	ld.type = LightType::DIRECTIONAL;
	ld.range = 1.0f;
	ld.intensity = 1.0f;
	ld.spot_angle = 20.0f;
	ld.color = vector3(1, 1, 1);
	stack.push_light_instance(stack.get_render_world(1)->create_light(stack.get_unit(2), ld, MATRIX4X4_IDENTITY));
	return 1;
}

static int render_world_destroy_light(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->destroy_light(stack.get_light_instance(2));
	return 0;
}

static int render_world_light_instances(lua_State* L)
{
	LuaStack stack(L);
	stack.push_light_instance(stack.get_render_world(1)->light(stack.get_unit(2)));
	return 1;
}

static int render_world_light_type(lua_State* L)
{
	LuaStack stack(L);
	LightType::Enum type = stack.get_render_world(1)->light_type(stack.get_light_instance(2));
	stack.push_string(s_light[type].name);
	return 1;
}

static int render_world_light_color(lua_State* L)
{
	LuaStack stack(L);
	stack.push_color4(stack.get_render_world(1)->light_color(stack.get_light_instance(2)));
	return 1;
}

static int render_world_light_range(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_range(stack.get_light_instance(2)));
	return 1;
}

static int render_world_light_intensity(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_intensity(stack.get_light_instance(2)));
	return 1;
}

static int render_world_light_spot_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.push_float(stack.get_render_world(1)->light_spot_angle(stack.get_light_instance(2)));
	return 1;
}

static int render_world_set_light_type(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_type(stack.get_light_instance(2)
		, name_to_light_type(stack, stack.get_string(3))
		);
	return 0;
}

static int render_world_set_light_color(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_color(stack.get_light_instance(2), stack.get_color4(3));
	return 0;
}

static int render_world_set_light_range(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_range(stack.get_light_instance(2), stack.get_float(3));
	return 0;
}

static int render_world_set_light_intensity(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_intensity(stack.get_light_instance(2), stack.get_float(3));
	return 0;
}

static int render_world_set_light_spot_angle(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->set_light_spot_angle(stack.get_light_instance(2), stack.get_float(3));
	return 0;
}

static int render_world_enable_debug_drawing(lua_State* L)
{
	LuaStack stack(L);
	stack.get_render_world(1)->enable_debug_drawing(stack.get_bool(2));
	return 0;
}

void load_render_world(LuaEnvironment& env)
{
	env.load_module_function("RenderWorld", "create_mesh",          render_world_create_mesh);
	env.load_module_function("RenderWorld", "destroy_mesh",         render_world_destroy_mesh);
	env.load_module_function("RenderWorld", "mesh_instances",       render_world_mesh_instances);
	env.load_module_function("RenderWorld", "mesh_obb",             render_world_mesh_obb);
	env.load_module_function("RenderWorld", "set_mesh_visible",     render_world_set_mesh_visible);
	env.load_module_function("RenderWorld", "create_sprite",        render_world_create_sprite);
	env.load_module_function("RenderWorld", "destroy_sprite",       render_world_destroy_sprite);
	env.load_module_function("RenderWorld", "sprite_instances",     render_world_sprite_instances);
	env.load_module_function("RenderWorld", "set_sprite_frame",     render_world_set_sprite_frame);
	env.load_module_function("RenderWorld", "set_sprite_visible",   render_world_set_sprite_visible);
	env.load_module_function("RenderWorld", "create_light",         render_world_create_light);
	env.load_module_function("RenderWorld", "destroy_light",        render_world_destroy_light);
	env.load_module_function("RenderWorld", "light_instances",      render_world_light_instances);
	env.load_module_function("RenderWorld", "light_type",           render_world_light_type);
	env.load_module_function("RenderWorld", "light_color",          render_world_light_color);
	env.load_module_function("RenderWorld", "light_range",          render_world_light_range);
	env.load_module_function("RenderWorld", "light_intensity",      render_world_light_intensity);
	env.load_module_function("RenderWorld", "light_spot_angle",     render_world_light_spot_angle);
	env.load_module_function("RenderWorld", "set_light_type",       render_world_set_light_type);
	env.load_module_function("RenderWorld", "set_light_color",      render_world_set_light_color);
	env.load_module_function("RenderWorld", "set_light_range",      render_world_set_light_range);
	env.load_module_function("RenderWorld", "set_light_intensity",  render_world_set_light_intensity);
	env.load_module_function("RenderWorld", "set_light_spot_angle", render_world_set_light_spot_angle);
	env.load_module_function("RenderWorld", "enable_debug_drawing", render_world_enable_debug_drawing);
}

} // namespace crown
