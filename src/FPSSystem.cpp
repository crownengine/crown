#include "InputManager.h"
#include "MovableCamera.h"
#include "FPSSystem.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat3.h"
#include "OS.h"

namespace crown
{

//-----------------------------------------------------------------------
FPSSystem::FPSSystem(MovableCamera* camera) :
	m_angle_x(0),
	m_angle_y(0),
	m_camera(camera),
	m_up_pressed(false),
	m_right_pressed(false),
	m_down_pressed(false),
	m_left_pressed(false)
{
//	get_input_manager()->register_keyboard_listener(this);
	get_input_manager()->register_accelerometer_listener(this);
	//get_input_manager()->register_mouse_listener(this);
	get_input_manager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
	
}

//-----------------------------------------------------------------------
void FPSSystem::key_pressed(const KeyboardEvent& event)
{
	switch (event.key)
	{
		case 'w':
		{
			m_up_pressed = true;
			break;
		}
		case 'a':
		{
			m_left_pressed = true;
			break;
		}
		case 's':
		{
			m_down_pressed = true;
			break;
		}
		case 'd':
		{
			m_right_pressed = true;
			break;
		}
		default:
		{
			break;
		}
	}
}

//-----------------------------------------------------------------------
void FPSSystem::key_released(const KeyboardEvent& event)
{
	switch (event.key)
	{
		case 'w':
		{
			m_up_pressed = false;
			break;
		}
		case 'a':
		{
			m_left_pressed = false;
			break;
		}
		case 's':
		{
			m_down_pressed = false;
			break;
		}
		case 'd':
		{
			m_right_pressed = false;
			break;
		}
		default:
		{
			break;
		}
	}
}

//-----------------------------------------------------------------------
void FPSSystem::accelerometer_changed(const AccelerometerEvent& event)
{
	set_view_by_cursor();
}
//-----------------------------------------------------------------------
void FPSSystem::set_camera(MovableCamera* camera)
{
	m_camera = camera;
}

//-----------------------------------------------------------------------
MovableCamera* FPSSystem::get_camera()
{
	return m_camera;
}

//-----------------------------------------------------------------------
void FPSSystem::camera_render()
{
	if (m_up_pressed)
	{
		m_camera->MoveForward();
	}

	if (m_left_pressed)
	{
		m_camera->StrafeLeft();
	}		

	if (m_down_pressed)
	{
		m_camera->MoveBackward();
	}

	if (m_right_pressed)
	{
		m_camera->StrafeRight();
	}

	m_camera->Render();
}

//-----------------------------------------------------------------------	
void FPSSystem::set_view_by_cursor()
{
	static Vec2 lastPos = get_input_manager()->get_cursor_relative_xy();
	Vec2 currentPos = get_input_manager()->get_cursor_relative_xy();
	get_input_manager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));

	if (lastPos == currentPos)
	{
		return;
	}

	Vec2 delta = lastPos - currentPos;

	get_input_manager()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
	lastPos = get_input_manager()->get_cursor_relative_xy();

	m_angle_x += delta.y * m_camera->GetSensibility();
	m_angle_y += delta.x * m_camera->GetSensibility();

	m_angle_x = math::clamp_to_range(-89.999f * math::DEG_TO_RAD, 89.999f * math::DEG_TO_RAD, m_angle_x);
	m_angle_y = math::fmod(m_angle_y, math::TWO_PI);

	m_camera->SetRotation(m_angle_x, m_angle_y);
}

} // namespace crown
