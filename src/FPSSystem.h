/*
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

#pragma once

#include "Types.h"
#include "Camera.h"
#include "Vec2.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Touch.h"
#include "Accelerometer.h"
#include "Renderer.h"

namespace crown
{
/// TODO: set_view_by_cursor must be implemented through scripting
class FPSSystem : public MouseListener, public KeyboardListener, public AccelerometerListener
{
public:

					/// Constructor
					FPSSystem(Camera* camera, float speed, float sensibility);

	void 			set_camera(Camera* camera);
	Camera*			camera();

	void			update(float dt);
	void			set_view_by_cursor();	

	virtual void 	key_pressed(const KeyboardEvent& event);
	virtual void 	key_released(const KeyboardEvent& event);
	virtual void 	accelerometer_changed(const AccelerometerEvent& event);

private:

	Camera*			m_camera;

	float			m_camera_speed;
	float			m_camera_sensibility;

	real 			m_angle_x;
	real 			m_angle_y;

	bool			m_up_pressed	: 1;
	bool			m_right_pressed	: 1;
	bool			m_down_pressed	: 1;
	bool			m_left_pressed	: 1;
};

} // namespace crown
