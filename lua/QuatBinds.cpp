#include "Quat.h"
#include "Vec3.h"
#include "LuaStack.h"
#include "LuaEnvironment.h"


namespace crown
{
extern "C"
{

const int32_t 	LUA_QUAT_BUFFER_SIZE = 4096;
Quat 			quat_buffer[LUA_QUAT_BUFFER_SIZE];
uint32_t 		quat_used = 0;

int32_t quat(lua_State* L)
{
	LuaStack stack(L);

	float w = stack.get_float(1);
	Vec3* v = (Vec3*)stack.get_lightudata(2);

	quat_buffer[quat_used].w = w;
	quat_buffer[quat_used].v = *v;

	stack.push_lightudata(&quat_buffer[quat_used]);

	quat_used++;

	return 1;
}

int32_t quat_negate(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);

	q->negate();

	return 0;
}

int32_t quat_load_identity(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);

	q->load_identity();

	return 0;
}

int32_t quat_length(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);

	stack.push_float(q->length());

	return 1;
}

int32_t quat_conjugate(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);

	Quat r = q->get_conjugate();

	stack.push_lightudata(&r);

	return 1;
}

int32_t quat_inverse(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);

	Quat r = q->get_inverse();

	stack.push_lightudata(&r);

	return 1;
}

int32_t quat_cross(lua_State* L)
{
	LuaStack stack(L);

	Quat* q1 = (Quat*)stack.get_lightudata(1);
	Quat* q2 = (Quat*)stack.get_lightudata(2);

	Quat r = *q1 * (*q2);

	stack.push_lightudata(&r);

	return 1;
}

int32_t quat_multiply(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);
	float k = stack.get_float(2);

	Quat r = *q * k;

	stack.push_lightudata(&r);

	return 1;
}

int32_t quat_power(lua_State* L)
{
	LuaStack stack(L);

	Quat* q = (Quat*)stack.get_lightudata(1);
	float k = stack.get_float(2);

	Quat r = q->power(k);

	stack.push_lightudata(&r);

	return 1;
}

} // extern "C"

void load_quat(LuaEnvironment& env)
{

}

} //namespace crown