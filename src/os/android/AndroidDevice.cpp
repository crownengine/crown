#include <jni.h>
#include "Device.h"

namespace crown
{

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj)
{
	device()->init(0, NULL);
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj)
{
	device()->shutdown();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj)
{
	device()->frame();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isInit(JNIEnv* env, jobject obj)
{
	return device()->is_init();
}

//-----------------------------------------------------------------------------
extern "C" JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isRunning(JNIEnv* env, jobject obj)
{
	return device()->is_running();
}

} // namespace crown
