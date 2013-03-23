#include "OS.h"
#include <jni.h>

namespace crown
{
namespace os
{

extern "C" 
{
    JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushEvent(JNIEnv * env, jobject obj, jint type, jint a, jint b, jint c, jint d);
};

JNIEXPORT void JNICALL Java_crown_android_CrownLib_pushEvent(JNIEnv * env, jobject obj, jint type, jint a, jint b, jint c, jint d)
{
	push_event((OSEventType)type, a, b, c, d);
}


void init_input()
{
	// FIXME
}

void get_cursor_xy(int32_t& x, int32_t& y)
{
	// not necessary
}

void set_cursor_xy(int32_t x, int32_t y)
{
	// not necessary
}

void hide_cursor()
{
	// not necessary
}

void show_cursor()
{
	// not necessary
}
} // namespace os
} // namespace crown
