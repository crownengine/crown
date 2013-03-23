#include <jni.h>
#include <sys/types.h>

namespace crown
{

extern "C" 
{
	// Renderer function...
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_beginFrame(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_endFrame(JNIEnv* env, jobject obj);
};

} // namespace crown
