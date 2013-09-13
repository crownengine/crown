/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "InputManager.h"
#include "Camera.h"
#include "FPSSystem.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat3.h"
#include "OS.h"
#include "Device.h"

namespace crown
{

//-----------------------------------------------------------------------
FPSSystem::FPSSystem(Camera* camera, float speed, float sensibility) :
	m_camera(camera),
	m_camera_speed(speed),
	m_camera_sensibility(sensibility),

	m_angle_x(0),
	m_angle_y(0),

	m_up_pressed(false),
	m_right_pressed(false),
	m_down_pressed(false),
	m_left_pressed(false)
{
	device()->input_manager()->register_keyboard_listener(this);
	device()->input_manager()->register_accelerometer_listener(this);
	//device()->input_manager()->register_mouse_listener(this);
	device()->mouse()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
	
}

//-----------------------------------------------------------------------
void FPSSystem::key_pressed(const KeyboardEvent& event)
{
	switch (event.key)
	{
		case 'w':
		case 'W':
		{
			m_up_pressed = true;
			break;
		}
		case 'a':
		case 'A':
		{
			m_left_pressed = true;
			break;
		}
		case 's':
		case 'S':
		{
			m_down_pressed = true;
			break;
		}
		case 'd':
		case 'D':
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
		case 'W':
		{
			m_up_pressed = false;
			break;
		}
		case 'a':
		case 'A':
		{
			m_left_pressed = false;
			break;
		}
		case 's':
		case 'S':
		{
			m_down_pressed = false;
			break;
		}
		case 'd':
		case 'D':
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
	(void)event;
	set_view_by_cursor();
}
//-----------------------------------------------------------------------
void FPSSystem::set_camera(Camera* camera)
{
	m_camera = camera;
}

//-----------------------------------------------------------------------
Camera* FPSSystem::camera()
{
	return m_camera;
}

//-----------------------------------------------------------------------
void FPSSystem::update(float dt)
{
	if (m_up_pressed)
	{
		m_camera->move_forward(m_camera_speed * dt);
	}

	if (m_left_pressed)
	{
		m_camera->strafe_left(m_camera_speed * dt);
	}		

	if (m_down_pressed)
	{
		m_camera->move_backward(m_camera_speed * dt);
	}

	if (m_right_pressed)
	{
		m_camera->strafe_right(m_camera_speed * dt);
	}

	// device()->renderer()->set_matrix(MT_VIEW, m_camera->view_matrix());
	// device()->renderer()->set_matrix(MT_PROJECTION, m_camera->projection_matrix());
}

//-----------------------------------------------------------------------	
void FPSSystem::set_view_by_cursor()
{
	static Vec2 lastPos = device()->mouse()->cursor_relative_xy();
	Vec2 currentPos = device()->mouse()->cursor_relative_xy();
	device()->mouse()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));

	if (lastPos == currentPos)
	{
		return;
	}

	Vec2 delta = lastPos - currentPos;

	device()->mouse()->set_cursor_relative_xy(Vec2(0.5f, 0.5f));
	lastPos = device()->mouse()->cursor_relative_xy();

	m_angle_x += delta.y * m_camera_sensibility;
	m_angle_y += delta.x * m_camera_sensibility;

	m_angle_x = math::clamp_to_range(-89.999f * math::DEG_TO_RAD, 89.999f * math::DEG_TO_RAD, m_angle_x);
	m_angle_y = math::fmod(m_angle_y, math::TWO_PI);

	m_camera->set_rotation(m_angle_x, m_angle_y);
}

} // namespace crown

