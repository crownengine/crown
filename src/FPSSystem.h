#pragma once
#include "Types.h"
#include "Camera.h"
#include "Vec2.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "Touch.h"
#include "Accelerometer.h"

namespace crown
{
/// TODO: GameplaySystem example class
class FPSSystem : public MouseListener, public KeyboardListener, public TouchListener, public AccelerometerListener
{
public:

					/// Constructor
					FPSSystem();

	void			set_camera_speed(const real speed);
	void 			set_camera_sensibility(const real sensibility);	
	void			camera_render();
	void			set_view_by_cursor();	

	virtual void 	KeyPressed(const KeyboardEvent& event);
	virtual void 	KeyReleased(const KeyboardEvent& event);

private:

	real 	m_angle_x;
	real 	m_angle_y;
	real	m_speed;
	real 	m_sensibility;
	
	MovableCamera* m_camera;

	bool m_up_pressed : 1;
	bool m_right_pressed : 1;
	bool m_down_pressed : 1;
	bool m_left_pressed : 1;
};

} // namespace crown
