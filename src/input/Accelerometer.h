#pragma once

namespace crown
{

class InputManager;

struct AccelerometerEvent
{
	float x;
	float y;
	float z;
};

class AccelerometerListener
{
public:
	virtual void accelerometer_changed(const AccelerometerEvent& event) { (void)event; }
};

class Accelerometer
{
public:
	
	Accelerometer() : m_listener(NULL) {}

	virtual ~Accelerometer() {}

	inline void set_listener(AccelerometerListener* listener) { m_listener = listener; }

private:

	AccelerometerListener* m_listener;
};

} // namespace crown
