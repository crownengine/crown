#include "Device.h"
#include "ScriptSystem.h"


namespace crown
{

extern "C"
{

	Vec2&				vec2(float nx, float ny);					

	Vec2&				vec2_add(Vec2& self, const Vec2& a);			

	Vec2& 				vec2_subtract(Vec2& self, const Vec2& a);			

	Vec2&				vec2_multiply(Vec2& self, float k);			

	Vec2&				vec2_divide(Vec2& self, float k);

	float				vec2_dot(Vec2& self, const Vec2& a);				

	bool				vec2_equals(Vec2& self, const Vec2& other);	

	bool				vec2_lower(Vec2& self, const Vec2& other);		

	bool				vec2_greater(Vec2& self, const Vec2& other);

	float				vec2_length(Vec2& self);

	float				vec2_squared_length(Vec2& self);

	void				vec2_set_length(Vec2& self, float len);					

	Vec2&				vec2_normalize(Vec2& self);

	Vec2&				vec2_negate(Vec2& self);

	float				vec2_get_distance_to(Vec2& self, const Vec2& a);

	float				vec2_get_angle_between(Vec2& self, const Vec2& a);

	void				vec2_zero(Vec2& self);
}

Vec2& vec2(float nx, float ny)
{
	return device()->script_system()->next_vec2(nx, ny);
}

Vec2& vec2_add(Vec2& self, const Vec2& a)
{
	self += a;

	return self;
}

Vec2& vec2_subtract(Vec2& self, const Vec2& a)
{
	self -= a;

	return self;
}

Vec2& vec2_multiply(Vec2& self, float k)
{
	self *= k;

	return self;
}

Vec2& vec2_divide(Vec2& self, float k)
{
	self /= k;

	return self;
}

float vec2_dot(Vec2& self, const Vec2& a)
{
	return self.dot(a);
}

bool vec2_equals(Vec2& self, const Vec2& other)
{
	return self == other;
}

bool vec2_lower(Vec2& self, const Vec2& other)
{
	return self < other;
}

bool vec2_greater(Vec2& self, const Vec2& other)
{
	return self > other;
}

float vec2_length(Vec2& self)
{
	self.length();
}

float vec2_squared_length(Vec2& self)
{
	return self.squared_length();
}

void vec2_set_length(Vec2& self, float len)
{
	self.set_length(len);
}

Vec2& vec2_normalize(Vec2& self)
{
	self.normalize();

	return self;
}

Vec2& vec2_negate(Vec2& self)
{
	self.negate();

	return self;
}

float vec2_get_distance_to(Vec2& self, const Vec2& a)
{
	return self.get_distance_to(a);
}

float vec2_get_angle_between(Vec2& self, const Vec2& a)
{
	return self.get_angle_between(a);
}

void vec2_zero(Vec2& self)
{
	self.zero();
}

} // namespace crown