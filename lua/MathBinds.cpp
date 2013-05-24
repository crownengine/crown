#include "MathUtils.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"

namespace crown
{

extern "C"
{

int32_t math_deg_to_rad(lua_State* L)
{
	LuaStack stack(L);

	float deg = stack.get_float(1);

	stack.push_float(math::deg_to_rad(deg));

	return 1;
}

int32_t math_rad_to_deg(lua_State* L)
{
	LuaStack stack(L);

	float rad = stack.get_float(1);

	stack.push_float(math::rad_to_deg(rad));

	return 1;
}

int32_t math_next_pow_2(lua_State* L)
{
	LuaStack stack(L);

	uint32_t x = stack.get_int(1);

	stack.push_int(math::next_pow_2(x));

	return 1;
}

int32_t math_is_pow_2(lua_State* L)
{
	LuaStack stack(L);

	uint32_t x = stack.get_int(1);

	stack.push_bool(math::is_pow_2(x));

	return 1;
}

int32_t math_ceil(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::ceil(x));

	return 1;
}

int32_t math_floor(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::floor(x));

	return 1;
}

int32_t math_sqrt(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::sqrt(x));

	return 1;
}

int32_t math_inv_sqrt(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::inv_sqrt(x));

	return 1;
}

int32_t math_sin(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::sin(x));

	return 1;
}

int32_t math_cos(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::cos(x));

	return 1;
}

int32_t math_asin(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::asin(x));

	return 1;
}

int32_t math_acos(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::acos(x));

	return 1;
}

int32_t math_tan(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::tan(x));

	return 1;
}

int32_t math_atan2(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);
	float y = stack.get_float(2);

	stack.push_float(math::atan2(x, y));

	return 1;
}

int32_t math_abs(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);

	stack.push_float(math::abs(x));

	return 1;
}

int32_t math_fmod(lua_State* L)
{
	LuaStack stack(L);

	float x = stack.get_float(1);
	float y = stack.get_float(2);

	stack.push_float(math::fmod(x, y));

	return 1;
}

} // extern "C"

void load_math(LuaEnvironment& env)
{
	env.load_module_function("Math", "deg_to_rad", math_deg_to_rad);
	env.load_module_function("Math", "rad_to_deg", math_rad_to_deg);
	env.load_module_function("Math", "next_pow_2", math_next_pow_2);
	env.load_module_function("Math", "is_pow_2", math_is_pow_2);
	env.load_module_function("Math", "ceil", math_ceil);
	env.load_module_function("Math", "floor", math_floor);
	env.load_module_function("Math", "sqrt", math_sqrt);
	env.load_module_function("Math", "inv_sqrt", math_inv_sqrt);
	env.load_module_function("Math", "sin", math_sin);
	env.load_module_function("Math", "cos", math_cos);
	env.load_module_function("Math", "asin", math_asin);
	env.load_module_function("Math", "acos", math_acos);
	env.load_module_function("Math", "tan", math_tan);
	env.load_module_function("Math", "atan2", math_atan2);
	env.load_module_function("Math", "abs", math_abs);
	env.load_module_function("Math", "fmod", math_fmod);
}

} // namespace crown