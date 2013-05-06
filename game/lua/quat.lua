require("vec3")

local ffi = require("ffi")

ffi.cdef
[[
	typedef struct
	{
		Vec3  v;
		float w;

	} Quat;

	Quat*		quat(float angle, const Vec3* v);

	void		quat_negate(Quat* self);

	void		quat_load_identity(Quat* self);

	float		quat_length(Quat* self);	

	void		quat_conjugate(Quat* self);

	Quat*		quat_inverse(Quat* self);			

	Quat*		quat_cross(Quat* self, const Quat* b);

	Quat*		quat_multiply(Quat* self, const float& k);

	Quat*		quat_power(Quat* self, float exp);
]]