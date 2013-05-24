#include "Vec3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

/// Lightweight bind for Vec3 

extern "C"
{

const int32_t 	LUA_VEC3_BUFFER_SIZE = 4096;
Vec3 			vec3_buffer[LUA_VEC3_BUFFER_SIZE];
uint32_t 		vec3_used = 0;

Vec3* next_vec3()
{
	return &vec3_buffer[vec3_used++];
}

//------------------------------------------------------------
int32_t vec3(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);
	float y = stack.get_float(2);
	float z = stack.get_float(3);

	vec3_buffer[vec3_used].x = x;
	vec3_buffer[vec3_used].y = y;
	vec3_buffer[vec3_used].z = z;

	stack.push_lightudata(&vec3_buffer[vec3_used]);

	vec3_used++;

	return 1;
}

//------------------------------------------------------------
int32_t vec3_values(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*)stack.get_lightudata(1);

	float x = a->x;
	float y = a->y;
	float z = a->z;

	stack.push_float(x);
	stack.push_float(y);
	stack.push_float(z);

	return 3;
}

//------------------------------------------------------------
int32_t vec3_add(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	*a += *b;

	stack.push_lightudata(a);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_subtract(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	*a -= *b;

	stack.push_lightudata(a);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_multiply(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	float b = stack.get_float(2);

	*a *= b;

	stack.push_lightudata(a);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_divide(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	float b = stack.get_float(2);

	*a /= b;

	stack.push_lightudata(a);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_dot(lua_State* L)
{
	LuaStack stack(L);
	
	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	stack.push_float(a->dot(*b));

	return 1;
}

//------------------------------------------------------------
int32_t vec3_cross(lua_State* L)
{
	LuaStack stack(L);
	
	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	/// CHECK CHECK CHECK
	*a = a->cross(*b);

	stack.push_lightudata(a);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_equals(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	stack.push_bool(*a == *b);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_lower(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	stack.push_bool(*a < *b);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_greater(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	stack.push_bool(*a > *b);

	return 1;
}

//------------------------------------------------------------
int32_t vec3_length(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);

	stack.push_float(a->length());

	return 1;
}

//------------------------------------------------------------
int32_t vec3_squared_length(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);

	stack.push_float(a->squared_length());

	return 1;
}

//------------------------------------------------------------
int32_t vec3_set_length(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	float len = stack.get_float(2);

	a->set_length(len);

	return 0;
}

//------------------------------------------------------------
int32_t vec3_normalize(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);

	a->normalize();

	return 0;
}

//------------------------------------------------------------
int32_t vec3_negate(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);

	a->negate();

	return 0;
}

//------------------------------------------------------------
int32_t vec3_get_distance_to(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	stack.push_float(a->get_distance_to(*b));

	return 1;
}

//------------------------------------------------------------
int32_t vec3_get_angle_between(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);
	Vec3* b = (Vec3*) stack.get_lightudata(2);

	stack.push_float(a->get_angle_between(*b));

	return 1;
}

//------------------------------------------------------------
int32_t vec3_zero(lua_State* L)
{
	LuaStack stack(L);

	Vec3* a = (Vec3*) stack.get_lightudata(1);

	a->zero();

	return 0;
}	

} // extern "C"

void load_vec3(LuaEnvironment& env)
{
	env.load_module_function("Vec3", "new", 				vec3);
	env.load_module_function("Vec3", "val", 				vec3_values);
	env.load_module_function("Vec3", "add", 				vec3_add);
	env.load_module_function("Vec3", "sub", 				vec3_subtract);
	env.load_module_function("Vec3", "mul", 				vec3_multiply);
	env.load_module_function("Vec3", "div", 				vec3_divide);
	env.load_module_function("Vec3", "dot", 				vec3_dot);
	env.load_module_function("Vec3", "cross", 				vec3_cross);
	env.load_module_function("Vec3", "equals", 				vec3_equals);
	env.load_module_function("Vec3", "lower", 				vec3_lower);
	env.load_module_function("Vec3", "greater", 			vec3_greater);
	env.load_module_function("Vec3", "length", 				vec3_length);
	env.load_module_function("Vec3", "squared_length", 		vec3_squared_length);
	env.load_module_function("Vec3", "set_length", 			vec3_set_length);
	env.load_module_function("Vec3", "normalize", 			vec3_normalize);
	env.load_module_function("Vec3", "negate", 				vec3_negate);
	env.load_module_function("Vec3", "get_distance_to", 	vec3_get_distance_to);
	env.load_module_function("Vec3", "get_angle_between", 	vec3_get_angle_between);
	env.load_module_function("Vec3", "zero", 				vec3_zero);	
}

} // namespace crown