#include <jni.h>
#include "Device.h"


namespace crown
{

extern "C" 
{
	//!< Device init() jni bind
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj);
	//!< Device shutdown() jni bind
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj);
};

//------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj)
{
	Device* device = GetDevice();
	device->Init(0, NULL);
}

//------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj)
{
	Device* device = GetDevice();
	device->Shutdown();
}

//------------------------------------------------------------------------------------
//JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj)
//{
//	GetDevice()->Frame();
//}

} // namspace crown
