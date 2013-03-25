#include <jni.h>
#include <sys/types.h>

namespace crown
{

extern "C" 
{
	//!< Device init() jni bind
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj, jint argc, jobjectArray argv);
	//!< Device is_running() jni bind	
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isRunning(JNIEnv* env, jobject obj);
	//!< Device shutdown() jni bind
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj);
	//!< Device frame() jni bind
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj);
};

} // namespace crown
