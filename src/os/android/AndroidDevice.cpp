#include "Device.h"
#include "AndroidDevice.h"

namespace crown
{

//------------------------------------------------------------------------------------
JNIEXPORT bool JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj, jint argc, jobject argv)
{
	return GetDevice()->Init(argc, (char**)argv);
}

//------------------------------------------------------------------------------------
JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isRunning(JNIEnv* env, jobject obj)
{
	return GetDevice()->IsRunning();
}

//------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj)
{
	GetDevice()->Shutdown();
}

} // namspace crown
