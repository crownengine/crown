#include "Quat.h"

namespace crown
{

/// Lightweight bind for Quat

extern "C"
{
	Quat*		quat(float angle, const Vec3* v);

	void		quat_negate(Quat* self);

	void		quat_load_identity(Quat* self);

	float		quat_length(Quat* self);	

	void		quat_conjugate(Quat* self);

	Quat*		quat_inverse(Quat* self);			

	Quat*		quat_cross(Quat* self, const Quat* b);

	Quat*		quat_multiply(Quat* self, const float& k);

	Quat*		quat_power(Quat* self, float exp);

	// Mat4		quat_to_mat4(Quat* self);
}

Quat* quat(float angle, const Vec3* v)
{
	return new Quat(angle, *v);
}

void quat_negate(Quat* self)
{
	self->negate();
}

void quat_load_identity(Quat* self)
{
	self->load_identity();
}

float quat_length(Quat* self)
{
	return self->length();
}

void quat_conjugate(Quat* self)
{
	self->conjugate();
}

Quat* quat_inverse(Quat* self)
{
	Quat tmp = self->get_inverse();

	return new Quat(tmp.w, tmp.v);
}

Quat* quat_cross(Quat* self, const Quat* b)
{
	*self = *self * (*b);

	return self;
}

Quat* quat_multiply(Quat* self, const float& k)
{
	*self = *self * k;

	return self;
}

Quat* quat_power(Quat* self, float exp)
{
	self->power(exp);

	return self;
}

} //namespace crown