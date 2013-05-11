#include "ScriptSystem.h"

namespace crown
{

/// Lightweight bind for Vec3 

extern "C"
{
	/// Constructor
	Vec3&				vec3(float nx, float ny, float nz);

	Vec3&				vec3_add(Vec3& self, const Vec3& v);

	Vec3&				vec3_subtract(Vec3& self, const Vec3& v);

	Vec3&				vec3_multiply(Vec3& self, const float s);

	Vec3&				vec3_divide(Vec3& self, const float s);

	float				vec3_dot(Vec3& self, const Vec3& v);

	Vec3&				vec3_cross(Vec3& self, const Vec3& v);

	bool				vec3_equal(Vec3& self, const Vec3& other);	
	
	bool				vec3_lower(Vec3& self, const Vec3& other);

	bool				vec3_greater(Vec3& self, const Vec3& other);		

	float				vec3_length(Vec3& self);	

	float				vec3_squared_length(Vec3& self);

	void				vec3_set_length(Vec3& self, float len);

	Vec3&				vec3_normalize(Vec3& self);

	Vec3&				vec3_negate(Vec3& self);					

	float				vec3_get_distance_to(Vec3& self, const Vec3& a);	

	float				vec3_get_angle_between(Vec3& self, const Vec3& a);	

	void				vec3_zero(Vec3& self);										
} // extern "C"

//------------------------------------------------------------
Vec3& vec3(float nx, float ny, float nz)
{
	return scripter()->next_vec3(nx, ny, nz);
}

//------------------------------------------------------------
Vec3& vec3_add(Vec3& self, const Vec3& v)
{
	self += v;

	return self;
}

//------------------------------------------------------------
Vec3& vec3_subtract(Vec3& self, const Vec3& v)
{
	self -= v;

	return self;
}

//------------------------------------------------------------
Vec3& vec3_multiply(Vec3& self, const float s)
{
	self *= s;

	return self;
}

//------------------------------------------------------------
Vec3& vec3_divide(Vec3& self, const float s)
{
	self /= s;

	return self;
}

//------------------------------------------------------------
float vec3_dot(Vec3& self, const Vec3& v)
{
	return self.dot(v);


}

//------------------------------------------------------------
Vec3& vec3_cross(Vec3& self, const Vec3& v)
{
	self.cross(v);

	return self;
}

//------------------------------------------------------------
bool vec3_equal(Vec3& self, const Vec3& other)
{
	return self == other;
}

//------------------------------------------------------------
bool vec3_lower(Vec3& self, const Vec3& other)
{
	return self < other;
}

//------------------------------------------------------------
bool vec3_greater(Vec3& self, const Vec3& other)
{
	return self > other;
}

//------------------------------------------------------------
float vec3_length(Vec3& self)
{
	return self.length();
}

//------------------------------------------------------------
float vec3_squared_length(Vec3& self)
{
	return self.squared_length();
}

//------------------------------------------------------------
void vec3_set_length(Vec3& self, float len)
{
	self.set_length(len);
}

//------------------------------------------------------------
Vec3& vec3_normalize(Vec3& self)
{
	self.normalize();

	return self;
}

//------------------------------------------------------------
Vec3& vec3_negate(Vec3& self)
{
	self.negate();

	return self;
}

//------------------------------------------------------------
float vec3_get_distance_to(Vec3& self, const Vec3& a)
{
	return self.get_distance_to(a);
}

//------------------------------------------------------------
float vec3_get_angle_between(Vec3& self, const Vec3& a)
{
	return self.get_angle_between(a);
}

//------------------------------------------------------------
void vec3_zero(Vec3& self)
{
	self.zero();
}	

} // namespace crown