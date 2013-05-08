local ffi = require("ffi")

ffi.cdef
[[	
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
]]

Math = {}

Math.equals 		= lib.math_equals
Math.test_bitmask 	= lib.math_test_bitmask
Math.set_bitmask 	= lib.math_set_bitmask
Math.unset_bitmask 	= lib.math_unset_bitmask	
Math.deg_to_rad 	= lib.math_deg_to_rad
Math.rad_to_deg 	= lib.math_rad_to_deg
Math.next_pow_2 	= lib.math_next_pow_2
Math.is_pow_2 		= lib.math_is_pow_2
Math.ceil 			= lib.math_ceil
Math.floor 			= lib.math_floor
Math.sqrt 			= lib.math_sqrt
Math.sin 			= lib.math_sin
Math.cos 			= lib.math_cos
Math.asin 			= lib.math_asin
Math.acos 			= lib.math_acos
Math.tan 			= lib.math_tan	
Math.atan2 			= lib.math_atan2
Math.abs 			= lib.math_abs
Math.fmod 			= lib.math_fmod