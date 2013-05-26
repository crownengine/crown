#include "Quat.h"
#include "Vec3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"


namespace crown
{

extern "C"
{

int32_t quat(lua_State* L)
{
	LuaStack stack(L);

	float w = stack.get_float(1);
	Vec3* v = stack.get_vec3(2);

	Quat* quat = next_quat();

	quat->w = w;
	quat->v = *v;

	stack.push_quat(quat);

	return 1;
}

int32_t quat_negate(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);

	q->negate();

	return 0;
}

int32_t quat_load_identity(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);

	q->load_identity();

	return 0;
}

int32_t quat_length(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);

	stack.push_float(q->length());

	return 1;
}

int32_t quat_conjugate(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);

	Quat* conjugate = next_quat();
	*conjugate = q->get_conjugate();

	stack.push_quat(conjugate);

	return 1;
}

int32_t quat_inverse(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);

	Quat* inverse = next_quat();
	*inverse = q->get_inverse();

	stack.push_quat(inverse);

	return 1;
}

int32_t quat_cross(lua_State* L)
{
	LuaStack stack(L);

	Quat* q1 = stack.get_quat(1);
	Quat* q2 = stack.get_quat(2);

	*q1 = (*q1) * (*q2);

	stack.push_quat(q1);

	return 1;
}

int32_t quat_multiply(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);
	float k = stack.get_float(2);

	*q = (*q) * k;

	stack.push_quat(q);

	return 1;
}

int32_t quat_power(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = stack.get_quat(1);
	float k = stack.get_float(2);

	q->power(k);

	stack.push_quat(q);

	return 1;
}

} // extern "C"

void load_quat(LuaEnvironment& env)
{
	env.load_module_function("Quat", "new", quat);
	env.load_module_function("Quat", "negate", quat_negate);
	env.load_module_function("Quat", "load_identity", quat_load_identity);
	env.load_module_function("Quat", "length", quat_length);
	env.load_module_function("Quat", "conjugate", quat_conjugate);
	env.load_module_function("Quat", "inverse", quat_inverse);
	env.load_module_function("Quat", "cross", quat_cross);
	env.load_module_function("Quat", "mul", quat_multiply);
	env.load_module_function("Quat", "pow", quat_power);
}

} //namespace crown