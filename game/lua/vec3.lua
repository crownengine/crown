local ffi = require("ffi")

ffi.cdef
[[
	typedef struct
	{
		float x;
		float y;
		float z;
	} Vec3;

	Vec3* 				vec3(float nx, float ny, float nz);

	Vec3*				vec3_add(Vec3* self, const Vec3* v);

	Vec3*				vec3_subtract(Vec3* self, const Vec3* v);

	Vec3*				vec3_multiply(Vec3* self, const float s);

	Vec3*				vec3_divide(Vec3* self, const float s);

	float				vec3_dot(Vec3* self, const Vec3* v);

	Vec3*				vec3_cross(Vec3* self, const Vec3* v);				

	bool				vec3_equal(Vec3* self, const Vec3* other);	
	
	bool				vec3_lower(Vec3* self, const Vec3* other);

	bool				vec3_greater(Vec3* self, const Vec3* other);		

	float				vec3_length(Vec3* self);	

	float				vec3_squared_length(Vec3* self);

	void				vec3_set_length(Vec3* self, float len);

	Vec3*				vec3_normalize(Vec3* self);

	Vec3*				vec3_negate(Vec3* self);					

	float				vec3_get_distance_to(Vec3* self, const Vec3* a);	

	float				vec3_get_angle_between(Vec3* self, const Vec3* a);	

	void				vec3_zero(Vec3* self);					
]]

-- Encapsulate method in Vec3 table

Vec3 = {}

Vec3.vec3 				= lib.vec3;
Vec3.add 				= lib.vec3_add
Vec3.subtract 			= lib.vec3_subtract
Vec3.multiply 			= lib.vec3_multiply
Vec3.divide 			= lib.vec3_divide
Vec3.dot 				= lib.vec3_dot
Vec3.cross 				= lib.vec3_cross
Vec3.equal 				= lib.vec3_equal
Vec3.lower 				= lib.vec3_lower
Vec3.greater 			= lib.vec3_greater
Vec3.lenght 			= lib.vec3_length
Vec3.squared_length 	= lib.vec3_squared_length
Vec3.set_length 		= lib.vec3_set_length
Vec3.normalize 			= lib.vec3_normalize
Vec3.negate 			= lib.vec3_negate
Vec3.get_distance_to 	= lib.vec3_get_distance_to
Vec3.get_angle_between 	= lib.vec3_get_angle_between
Vec3.zero 				= lib.vec3_zero