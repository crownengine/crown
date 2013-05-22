#include "MathUtils.h"
#include "Types.h"

namespace crown
{

extern "C"
{
	bool						math_equals(float a, float b);

	bool						math_test_bitmask(int32_t value, int32_t bitmask);

	int32_t						math_set_bitmask(int32_t value, int32_t bitmask);

	int32_t						math_unset_bitmask(int32_t value, int32_t bitmask);					

	float						math_deg_to_rad(float deg);

	float						math_rad_to_deg(float rad);

	uint32_t					math_next_pow_2(uint32_t x);

	bool						math_is_pow_2(uint32_t x);	

	float						math_ceil(float x);		

	float						math_floor(float x);	

	float						math_sqrt(float x);	

	float						math_inv_sqrt(float x);

	float						math_sin(float x);	

	float						math_cos(float x);

	float						math_asin(float x);	

	float						math_acos(float x);	

	float						math_tan(float x);		

	float						math_atan2(float y, float x);	

	float						math_abs(float x);			

	float						math_fmod(float n, float d);			
}

//-------------------------------------------------------------------
bool math_equals(float a, float b)
{
	return math::equals(a, b, math::FLOAT_PRECISION);
}

//-------------------------------------------------------------------
bool math_test_bitmask(int32_t value, int32_t bitmask)
{
	return math::test_bitmask(value, bitmask);
}

//-------------------------------------------------------------------
int32_t math_set_bitmask(int32_t value, int32_t bitmask)
{
	return math::set_bitmask(value, bitmask);
}

//-------------------------------------------------------------------
int32_t math_unset_bitmask(int32_t value, int32_t bitmask)
{
	return math::unset_bitmask(value, bitmask);
}

//-------------------------------------------------------------------
float math_deg_to_rad(float deg)
{
	return math::deg_to_rad(deg);
}

//-------------------------------------------------------------------
float math_rad_to_deg(float rad)
{
	return math::rad_to_deg(rad);
}

//-------------------------------------------------------------------
uint32_t math_next_pow_2(uint32_t x)
{
	return math::next_pow_2(x);
}

//-------------------------------------------------------------------
bool math_is_pow_2(uint32_t x)
{
	return math::is_pow_2(x);
}

//-------------------------------------------------------------------
float math_ceil(float x)
{
	return math::ceil(x);
}

//-------------------------------------------------------------------
float math_floor(float x)
{
	return math::floor(x);
}

//-------------------------------------------------------------------
float math_sqrt(float x)
{
	return math::sqrt(x);
}

//-------------------------------------------------------------------
float math_inv_sqrt(float x)
{
	return math::inv_sqrt(x);
}

//-------------------------------------------------------------------
float math_sin(float x)
{
	return math::sin(x);
}

//-------------------------------------------------------------------
float math_cos(float x)
{
	return math::cos(x);
}

//-------------------------------------------------------------------
float math_asin(float x)
{
	return math::asin(x);
}

//-------------------------------------------------------------------
float math_acos(float x)
{
	return math::acos(x);
}

//-------------------------------------------------------------------
float math_tan(float x)
{
	return math::tan(x);
}

//-------------------------------------------------------------------
float math_atan2(float y, float x)
{
	return math::atan2(y, x);
}

//-------------------------------------------------------------------
float math_abs(float x)
{
	return math::abs(x);
}

//-------------------------------------------------------------------
float math_fmod(float n, float d)
{
	return math::fmod(n, d);
}

//-------------------------------------------------------------------
// bool math_solve_quadratic_equation(float a, float b, float c, float& x1, float& x2)
// {
// 	return math::solve_quadratic_equation(a, b, c, x1, x2);
// }

} // namespace crown