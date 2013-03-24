#pragma once

namespace crown
{

class InputManager;

struct AccelerometerEvent
{
	int32_t x;
	int32_t y;
	int32_t z;
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
