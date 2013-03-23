#include "AndroidRenderer.h"
#include "Device.h"
#include "Renderer.h"

namespace crown
{

JNIEXPORT void JNICALL Java_crown_android_CrownLib_beginFrame(JNIEnv* env, jobject obj)
{
	GetDevice()->GetRenderer()->_BeginFrame();
}

JNIEXPORT void JNICALL Java_crown_android_CrownLib_endFrame(JNIEnv* env, jobject obj)
{
	GetDevice()->GetRenderer()->_EndFrame();
}

} // namespace crown
