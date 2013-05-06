#include "Mat4.h"

namespace crown
{

extern "C"
{
	Mat4*				mat4(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2, float r1c3, float r2c3, float r3c3);
						
	Mat4*				mat4_add(Mat4* self, Mat4* m);

	Mat4*				mat4_subtract(Mat4* self, Mat4* m);

	Mat4*				mat4_multiply(Mat4* self, Mat4* m);

	Mat4*				mat4_multiply_by_scalar(Mat4* self, float k);

	Mat4*				mat4_divide_by_scalar(Mat4* self, float k);

	void				mat4_build_rotation_x(Mat4* self, float radians);

	void				mat4_build_rotation_y(Mat4* self, float radians);	

	void				mat4_build_rotation_z(Mat4* self, float radians);

	void				mat4_build_rotation(Mat4* self, const Vec3* n, float radians);

	void				mat4_build_projection_perspective_rh(Mat4* self, float fovy, float aspect, float near, float far);

	void				mat4_build_projection_perspective_lh(Mat4* self, float fovy, float aspect, float near, float far);

	void				mat4_build_projection_ortho_rh(Mat4* self, float width, float height, float near, float far);

	void				mat4_build_projection_ortho_lh(Mat4* self, float width, float height, float near, float far);

	void				mat4_build_projection_ortho_2d_rh(Mat4* self, float width, float height, float near, float far);

	void				mat4_build_look_at_rh(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up);

	void				mat4_build_look_at_lh(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up);

	void				mat4_build_viewpoint_billboard(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up);

	void				mat4_build_axis_billboard(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* axis);

	Mat4*				mat4_transpose(Mat4* self);

	float				mat4_determinant(Mat4* self);

	Mat4*				mat4_invert(Mat4* self);

	void				mat4_load_identity(Mat4* self);

	Vec3*				mat4_get_translation(Mat4* self);

	void				mat4_set_translation(Mat4* self, const Vec3* trans);

	Vec3*				mat4_get_scale(Mat4* self);

	void				mat4_set_scale(Mat4* self, const Vec3* scale);

	// Quat				mat4_to_quat();
}

Mat4* mat4(float r1c1, float r2c1, float r3c1, float r1c2, float r2c2, float r3c2, float r1c3, float r2c3, float r3c3)
{
	return new Mat4(r1c1, r2c1, r3c1, 0, r1c2, r2c2, r3c2, 0, r1c3, r2c3, r3c3, 0, 0, 0, 0, 1);
}
					
Mat4* mat4_add(Mat4* self, Mat4* m)
{
	*self += *m;

	return self;
}

Mat4* mat4_subtract(Mat4* self, Mat4* m)
{
	*self -= *m;

	return self;
}

Mat4* mat4_multiply(Mat4* self, Mat4* m)
{
	*self *= *m;

	return self;
}	

Mat4* mat4_multiply_by_scalar(Mat4* self, float k)
{
	*self *= k;

	return self;
}

Mat4* mat4_divide_by_scalar(Mat4* self, float k)
{
	*self /= k;

	return self;
}

void mat4_build_rotation_x(Mat4* self, float radians)
{
	self->build_rotation_x(radians);
}

void mat4_build_rotation_y(Mat4* self, float radians)
{
	self->build_rotation_y(radians);
}

void mat4_build_rotation_z(Mat4* self, float radians)
{
	self->build_rotation_z(radians);
}

void mat4_build_rotation(Mat4* self, const Vec3* n, float radians)
{
	self->build_rotation(*n, radians);
}

void mat4_build_projection_perspective_rh(Mat4* self, float fovy, float aspect, float near, float far)
{
	self->build_projection_perspective_rh(fovy, aspect, near, far);
}

void mat4_build_projection_perspective_lh(Mat4* self, float fovy, float aspect, float near, float far)
{
	self->build_projection_perspective_lh(fovy, aspect, near, far);
}

void mat4_build_projection_ortho_rh(Mat4* self, float width, float height, float near, float far)
{
	self->build_projection_ortho_rh(width, height, near, far);
}

void mat4_build_projection_ortho_lh(Mat4* self, float width, float height, float near, float far)
{
	self->build_projection_ortho_lh(width, height, near, far);
}

void mat4_build_projection_ortho_2d_rh(Mat4* self, float width, float height, float near, float far)
{
	self->build_projection_ortho_2d_rh(width, height, near, far);
}

void mat4_build_look_at_rh(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up)
{
	self->build_look_at_rh(*pos, *target, *up);
}

void mat4_build_look_at_lh(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up)
{
	self->build_look_at_lh(*pos, *target, *up);
}

void mat4_build_viewpoint_billboard(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* up)
{
	self->build_viewpoint_billboard(*pos, *target, *up);
}

void mat4_build_axis_billboard(Mat4* self, const Vec3* pos, const Vec3* target, const Vec3* axis)
{
	self->build_axis_billboard(*pos, *target, *up);
}

Mat4* mat4_transpose(Mat4* self)
{
	self->transpose();

	return self;
}

float mat4_determinant(Mat4* self)
{
	return self->get_determinant();
}

Mat4* mat4_invert(Mat4* self)
{
	self->invert();

	return self;
}

void mat4_load_identity(Mat4* self)
{
	self->load_identity();
}

Vec3* mat4_get_translation(Mat4* self)
{
	Vec3 tmp = self->get_translation();

	return &tmp;
}

void mat4_set_translation(Mat4* self, const Vec3* trans)
{
	self->set_translation(*trans);
}

Vec3* mat4_get_scale(Mat4* self)
{
	Vec3 tmp = self->get_scale();

	return &tmp;
}

void mat4_set_scale(Mat4* self, const Vec3* scale)
{
	self->set_scale(*scale);
}

} //namespace crown