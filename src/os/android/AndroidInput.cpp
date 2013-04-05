#include "OS.h"
#include "Log.h"
#include <jni.h>

namespace crown
{
namespace os
{

extern "C" 
{
	/// OS push_event() jni bind
    JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushIntEvent(JNIEnv * env, jobject obj, jint type, jint a, jint b, jint c, jint d);
    JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushFloatEvent(JNIEnv * env, jobject obj, jint type, jfloat a, jfloat b, jfloat c, jfloat d);    
};

//-----------------------------------------------------------------------------
void init_input()
{
	// FIXME
}

//-----------------------------------------------------------------------------
void event_loop()
{
	// not necessary, implemented in Android
}

//-----------------------------------------------------------------------------
void get_cursor_xy(int32_t& x, int32_t& y)
{
	// not necessary
}

//-----------------------------------------------------------------------------
void set_cursor_xy(int32_t x, int32_t y)
{
	// not necessary
}

//-----------------------------------------------------------------------------
void hide_cursor()
{
	// not necessary
}

//-----------------------------------------------------------------------------
void show_cursor()
{
	// not necessary
}

//-----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushIntEvent(JNIEnv * env, jobject obj, jint type, jint a, jint b, jint c, jint d)
{	
	OSEventParameter values[4];

	values[0].int_value = a;
	values[1].int_value = b;
	values[2].int_value = c;
	values[3].int_value = d;

	push_event((OSEventType)type, values[0], values[1], values[2], values[3]);
}

//-----------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushFloatEvent(JNIEnv * env, jobject obj, jint type, jfloat a, jfloat b, jfloat c, jfloat d)
{
	OSEventParameter values[4];

	values[0].float_value = a;
	values[1].float_value = b;
	values[2].float_value = c;
	values[3].float_value = d;


	push_event((OSEventType)type, values[0], values[1], values[2], values[3]);
}

} // namespace os
} // namespace crown
