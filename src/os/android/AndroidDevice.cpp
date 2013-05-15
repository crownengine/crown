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
	device()->init(0, NULL);
}

//------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj)
{
	device()->shutdown();
}

} // namespace crown
