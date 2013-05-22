#pragma once

#include "lua.hpp"
#include "Types.h"

namespace crown
{

class LuaStack
{
public:

							LuaStack(lua_State* L);

	void					push_bool(bool value);

	void					push_int(int32_t value);

	void 					push_float(float value);

	void 					push_string(const char* str, size_t len);

	void					push_lightudata(void* ptr);

	bool 					get_bool(int32_t index);

	int32_t					get_int(int32_t index);

	float 					get_float(int32_t index);

	const char*				get_string(int32_t index);

private:

	lua_State* 				m_state;
};

} // namespace crown