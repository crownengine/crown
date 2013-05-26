#include "Camera.h"

namespace crown
{

extern "C"
{

Camera*				camera(const Vec3& position, float fov, float aspect);

const Vec3&			camera_position(Camera* self);

void				camera_set_position(Camera* self, const Vec3& position);

const Vec3&			camera_look_at(Camera* self);

void				camera_set_look_at(Camera* self, const Vec3& lookat);

void				camera_set_rotation(Camera* self, const float x, const float y);

const Vec3&			camera_up(Camera* self);

float				camera_fov(Camera* self);

void				camera_set_fov(Camera* self, float fov);

float				camera_aspect(Camera* self);

void				camera_set_aspect(Camera* self, float aspect);

float				camera_near_clip_distance(Camera* self);

void				camera_set_near_clip_distance(Camera* self, float near);

float				camera_far_clip_distance(Camera* self);

void				camera_set_far_clip_distance(Camera* self, float far);

const Mat4&			camera_projection_matrix(Camera* self);

const Mat4&			camera_view_matrix(Camera* self);

void				camera_move_forward(Camera* self, float meters);

void				camera_move_backward(Camera* self, float meters);

void				camera_strafe_left(Camera* self, float meters);

void				camera_strafe_right(Camera* self, float meters);
}

Camera* camera(const Vec3& position, float fov, float aspect)
{
	return new Camera(position, fov, aspect);
}

const Vec3& camera_position(Camera* self)
{
	return self->position();
}

void camera_set_position(Camera* self, const Vec3& position)
{
	self->set_position(position);
}

const Vec3& camera_look_at(Camera* self)
{
	return self->look_at();
}

void camera_set_look_at(Camera* self, const Vec3& lookat)
{
	self->set_look_at(lookat);
}

void camera_set_rotation(Camera* self, const float x, const float y)
{
	self->set_rotation(x, y);
}

const Vec3& camera_up(Camera* self)
{
	return self->up();
}

float camera_fov(Camera* self)
{
	return self->fov();
}

void camera_set_fov(Camera* self, float fov)
{
	self->set_fov(fov);
}

float camera_aspect(Camera* self)
{
	return self->aspect();
}

void camera_set_aspect(Camera* self, float aspect)
{
	self->set_aspect(aspect);
}

float camera_near_clip_distance(Camera* self)
{
	return self->near_clip_distance();
}

void camera_set_near_clip_distance(Camera* self, float near)
{
	self->set_near_clip_distance(near);
}

float camera_far_clip_distance(Camera* self)
{
	return self->far_clip_distance();
}

void camera_set_far_clip_distance(Camera* self, float far)
{
	self->set_far_clip_distance(far);
}

const Mat4& camera_projection_matrix(Camera* self)
{
	return self->projection_matrix();
}

const Mat4& camera_view_matrix(Camera* self)
{
	return self->view_matrix();
}

void camera_move_forward(Camera* self, float meters)
{
	self->move_forward(meters);
}

void camera_move_backward(Camera* self, float meters)
{
	self->move_backward(meters);
}

void camera_strafe_left(Camera* self, float meters)
{
	self->strafe_left(meters);
}

void camera_strafe_right(Camera* self, float meters)
{
	self->strafe_right(meters);
}
} // namespace crown