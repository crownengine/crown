#include "Crown.h"
#include "EventBuffer.h"
#include "OsTypes.h"
#include "Mouse.h"
#include "Log.h"

using namespace crown;

//-----------------------------------------------------------------------------
int main()
{
	EventBuffer buffer;

	OsMouseEvent ome;
	ome.button = MouseButton::LEFT;
	ome.x = 0;
	ome.y = 0;
	ome.pressed = true;

	buffer.push_event((uint32_t)OsEvent::MOUSE, &ome, sizeof(OsMouseEvent));

	OsEvent::Enum type = (OsEvent::Enum)buffer.get_next_event_type();
	Log::d("type: %d", type);

	OsMouseEvent* result;
	uint32_t et; size_t es;
	result = (OsMouseEvent*)buffer.get_next_event(et, es);

	Log::d("button: %d", result->button);
	Log::d("x: %d", result->x);
	Log::d("y: %d", result->y);
	Log::d("pressed: %d", result->pressed);

	return 0;
}