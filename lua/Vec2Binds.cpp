#include "LuaStack.h"
#include "Device.h"
#include "ScriptSystem.h"
#include "Vec2Binds.h"
#include "OS.h"

namespace crown
{

extern "C"
{

int32_t	vec2(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);
	float y = stack.get_float(2);

	stack.push_lightudata(device()->script_system()->next_vec2(x, y));

	return 1;
}

int32_t vec2_add(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	*a += *b;

	stack.push_lightudata(a);

	return 1;
}

int32_t vec2_subtract(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	*a -= *b;

	stack.push_lightudata(a);

	return 1;
}

int32_t vec2_multiply(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);
	float k = stack.get_float(2);

	*a *= k;

	stack.push_lightudata(a);

	return 1;
}			
	
int32_t vec2_divide(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	float k = stack.get_float(2);

	*a /= k;

	stack.push_lightudata(a);

	return 1;
}

int32_t vec2_dot(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	stack.push_float(a->dot(*b));

	return 1;
}

int32_t vec2_equals(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	stack.push_bool(*a == *b);

	return 1;
}

int32_t vec2_lower(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	stack.push_bool(*a < *b);

	return 1;
}

int32_t vec2_greater(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);
	
	stack.push_bool(*a > *b);

	return 1;
}

int32_t vec2_length(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);

	stack.push_float(a->length());

	return 1;
}

int32_t vec2_squared_length(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);

	stack.push_float(a->squared_length());

	return 1;
}

int32_t vec2_set_length(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);
	float len = stack.get_float(2);

	a->set_length(len);

	return 0;
}

int32_t	vec2_normalize(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);

	a->normalize();

	stack.push_lightudata(a);

	return 1;
}

int32_t	vec2_negate(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);

	a->negate();

	stack.push_lightudata(a);

	return 1;
}

int32_t	vec2_get_distance_to(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	stack.push_float(a->get_distance_to(*b));

	return 1;
}

int32_t	vec2_get_angle_between(lua_State* L)
{
	LuaStack stack(L);
	
	Vec2* a = (Vec2*) stack.get_lightudata(1);
	Vec2* b = (Vec2*) stack.get_lightudata(2);

	stack.push_float(a->get_angle_between(*b));

	return 1;
}

int32_t	vec2_zero(lua_State* L)
{
	LuaStack stack(L);

	Vec2* a = (Vec2*) stack.get_lightudata(1);

	a->zero();

	return 0;
}

static const struct luaL_Reg Vec2 [] = {
	{"new", 			vec2},
	{"add", 			vec2_add},
	{"sub", 			vec2_subtract},
	{"mul", 			vec2_multiply},
	{"div", 			vec2_divide},
	{"dot", 			vec2_dot},
	{"equals", 			vec2_equals},
	{"lower", 			vec2_lower},
	{"greater", 		vec2_greater},
	{"len", 			vec2_length},
	{"squared_len", 	vec2_squared_length},
	{"set_len", 		vec2_set_length},
	{"norm", 			vec2_normalize},
	{"neg", 			vec2_negate},
	{"dist_to", 		vec2_get_distance_to},
	{"angle_between", 	vec2_get_angle_between},
	{"zero", 			vec2_zero},
	{NULL, NULL}	
};

int32_t luaopen_libcrownlua(lua_State* L)
{
	luaL_register(L, "Vec2", Vec2);
	return 1;
}

}

} // namespace crown