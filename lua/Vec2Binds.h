#pragma once

#include "Vec2.h"
#include "lua.hpp"

namespace crown
{

extern "C"
{
	/// Constructor
	int32_t				vec2(lua_State* L);					
	/// a + b
	int32_t				vec2_add(lua_State* L);			
	/// a - b
	int32_t 			vec2_subtract(lua_State* L);			
	///
	int32_t				vec2_multiply(lua_State* L);			
	///
	int32_t				vec2_divide(lua_State* L);
	///
	int32_t				vec2_dot(lua_State* L);				
	///
	int32_t				vec2_equals(lua_State* L);	
	///
	int32_t				vec2_lower(lua_State* L);		
	///
	int32_t				vec2_greater(lua_State* L);
	///
	int32_t				vec2_length(lua_State* L);
	///
	int32_t				vec2_squared_length(lua_State* L);
	///
	int32_t				vec2_set_length(lua_State* L);					
	///
	int32_t				vec2_normalize(lua_State* L);
	///
	int32_t				vec2_negate(lua_State* L);
	///
	int32_t				vec2_get_distance_to(lua_State* L);
	///
	int32_t				vec2_get_angle_between(lua_State* L);
	///
	int32_t				vec2_zero(lua_State* L);
}

} // namespace crown