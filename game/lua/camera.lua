local ffi = require("ffi")

ffi.cdef
[[
	typedef struct 
	{
		Vec3			m_position;
		Vec3			m_look_at;
		Vec3			m_up;

		float			m_angle_x;
		float			m_angle_y;

		Mat4			m_view;
		Mat4			m_projection;

		float			m_FOV;
		float			m_aspect;

		float			m_near;
		float			m_far;
	} Camera;

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

]]

Camera = {}

Camera.camera 					= lib.camera
Camera.position					= lib.camera_position
Camera.set_position				= lib.camera_set_position
Camera.look_at					= lib.camera_look_at
Camera.set_look_at				= lib.camera_set_look_at
Camera.set_rotation				= lib.camera_set_rotation
Camera.up						= lib.camera_up
Camera.fov						= lib.camera_fov
Camera.set_fov					= lib.camera_set_fov
Camera.aspect					= lib.camera_aspect
Camera.set_aspect				= lib.camera_set_aspect
Camera.near_clip_distance		= lib.camera_near_clip_distance
Camera.set_near_clip_distance	= lib.camera_set_near_clip_distance
Camera.far_clip_distance		= lib.camera_far_clip_distance
Camera.set_far_clip_distance	= lib.camera_set_far_clip_distance
Camera.projection_matrix		= lib.camera_projection_matrix
Camera.view_matrix				= lib.camera_view_matrix
Camera.move_forward				= lib.camera_move_forward
Camera.move_backward			= lib.camera_move_backward
Camera.strafe_left				= lib.camera_strafe_left
Camera.strafe_right				= lib.camera_strafe_right
