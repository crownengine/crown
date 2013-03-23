#include <jni.h>
#include <sys/types.h>

namespace crown
{

extern "C" 
{
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj, jint argc, jobjectArray argv);
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_isRunning(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj);
};

} // namespace crown
