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

local lib_path = os.getenv("LD_LIBRARY_PATH")

lib = ffi.load(lib_path .. "libcrown.so", true)


local dir = lib.vec3(1, 1, 1)

print(lib.vec3_negate(dir).x)



