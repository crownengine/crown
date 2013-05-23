#include "MathUtils.h"
#include "Types.h"
#include "LuaStack.h"
#include "OS.h"

namespace crown
{

extern "C"
{
	// int32_t						math_test_bitmask(lua_State* L, int32_t value, int32_t bitmask);

	// int32_t						math_set_bitmask(lua_State* L, int32_t value, int32_t bitmask);

	// int32_t						math_unset_bitmask(lua_State* L, int32_t value, int32_t bitmask);					

	// int32_t						math_deg_to_rad(lua_State* L, float deg);

	// int32_t						math_rad_to_deg(lua_State* L, float rad);

	// int32_t						math_next_pow_2(lua_State* L, uint32_t x);

	// int32_t						math_is_pow_2(lua_State* L, uint32_t x);	

	// int32_t						math_ceil(lua_State* L, float x);		

	// int32_t						math_floor(lua_State* L, float x);	

	// int32_t						math_sqrt(lua_State* L, float x);	

	// int32_t						math_inv_sqrt(lua_State* L, float x);

	// int32_t						math_sin(lua_State* L, float x);	

	// int32_t						math_cos(lua_State* L, float x);

	// int32_t						math_asin(lua_State* L, float x);	

	// int32_t						math_acos(lua_State* L, float x);	

	// int32_t						math_tan(lua_State* L, float x);		

	// int32_t						math_atan2(lua_State* L, float y, float x);	

	// int32_t						math_abs(lua_State* L, float x);			

	// int32_t						math_fmod(lua_State* L, float n, float d);			

//-------------------------------------------------------------------
int32_t math_equals(lua_State* L)
{
	LuaStack stack(L);

	float a = stack.get_float(1);
	float b = stack.get_float(2);

	stack.push_bool(math::equals(a, b, math::FLOAT_PRECISION));

	return 1;
}

// //-------------------------------------------------------------------
// int32_t math_test_bitmask(lua_State* L, int32_t value, int32_t bitmask)
// {
// 	return math::test_bitmask(value, bitmask);
// }

// //-------------------------------------------------------------------
// int32_t math_set_bitmask(lua_State* L, int32_t value, int32_t bitmask)
// {
// 	return math::set_bitmask(value, bitmask);
// }

// //-------------------------------------------------------------------
// int32_t math_unset_bitmask(lua_State* L, int32_t value, int32_t bitmask)
// {
// 	return math::unset_bitmask(value, bitmask);
// }

// //-------------------------------------------------------------------
// int32_t math_deg_to_rad(lua_State* L, float deg)
// {
// 	return math::deg_to_rad(deg);
// }

// //-------------------------------------------------------------------
// inatt32_t math_rad_to_deg(lua_State* L, float rad)
// {
// 	return math::rad_to_deg(rad);
// }

// //-------------------------------------------------------------------
// int32_t math_next_pow_2(lua_State* L, uint32_t x)
// {
// 	return math::next_pow_2(x);
// }

// //-------------------------------------------------------------------
// int32_t math_is_pow_2(lua_State* L, uint32_t x)
// {
// 	return math::is_pow_2(x);
// }

// //-------------------------------------------------------------------
// int32_t math_ceil(lua_State* L, float x)
// {
// 	return math::ceil(x);
// }

// //-------------------------------------------------------------------
// int32_t math_floor(lua_State* L, float x)
// {
// 	return math::floor(x);
// }

// //-------------------------------------------------------------------
// int32_t math_sqrt(lua_State* L, float x)
// {
// 	return math::sqrt(x);
// }

// //-------------------------------------------------------------------
// int32_t math_inv_sqrt(lua_State* L, float x)
// {
// 	return math::inv_sqrt(x);
// }

// //-------------------------------------------------------------------
// int32_t math_sin(lua_State* L, float x)
// {
// 	return math::sin(x);
// }

// //-------------------------------------------------------------------
// int32_t math_cos(lua_State* L, float x)
// {
// 	return math::cos(x);
// }

// //-------------------------------------------------------------------
// int32_t math_asin(lua_State* L, float x)
// {
// 	return math::asin(x);
// }

// //-------------------------------------------------------------------
// int32_t math_acos(lua_State* L, float x)
// {
// 	return math::acos(x);
// }

// //-------------------------------------------------------------------
// int32_t math_tan(lua_State* L, float x)
// {
// 	return math::tan(x);
// }

// //-------------------------------------------------------------------
// int32_t math_atan2(lua_State* L, float y, float x)
// {
// 	return math::atan2(y, x);
// }

// //-------------------------------------------------------------------
// int32_t math_abs(lua_State* L, float x)
// {
// 	return math::abs(x);
// }

// //-------------------------------------------------------------------
// int32_t math_fmod(lua_State* L, float n, float d)
// {
// 	return math::fmod(n, d);
// }

static const struct luaL_Reg Math [] = {
	{"equals", math_equals},
	{NULL, NULL}	
};

}

} // namespace crown