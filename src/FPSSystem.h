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
/// TODO: set_view_by_cursor must be implemented through scripting
class FPSSystem : public MouseListener, public KeyboardListener, public AccelerometerListener
{
public:

					/// Constructor
					FPSSystem(MovableCamera* camera);

	void 			set_camera(MovableCamera* camera);
	MovableCamera*	get_camera();

	void			camera_render();
	void			set_view_by_cursor();	

	virtual void 	key_pressed(const KeyboardEvent& event);
	virtual void 	key_released(const KeyboardEvent& event);
	virtual void 	accelerometer_changed(const AccelerometerEvent& event);

private:

	real 	m_angle_x;
	real 	m_angle_y;

	bool m_up_pressed : 1;
	bool m_right_pressed : 1;
	bool m_down_pressed : 1;
	bool m_left_pressed : 1;

	MovableCamera* m_camera;
};

} // namespace crown
