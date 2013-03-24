#include "Crown.h"
#include "OS.h"
#include <jni.h>
#include <GLES/gl.h>

namespace crown
{

extern "C"
{
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj);
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj);
	JNIEXPORT bool JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj);
};

void draw_triangle()
{
	static GLfloat vertices[] = {	-1.0f, -1.0f, -2.0f,
									1.0f, -1.0f, -2.0f,
									0.0f, 1.0f, -2.0f};

	GetDevice()->GetRenderer()->SetMatrix(MT_MODEL, Mat4::IDENTITY);

	Mat4 projection;
	projection.build_projection_perspective_rh(90.0f, 800.0f/480.0f, 0.1f, 100.0f);
	GetDevice()->GetRenderer()->SetMatrix(MT_PROJECTION, projection);

	GetDevice()->GetRenderer()->SetClearColor(Color4::RED);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	glDrawArrays(GL_TRIANGLES, 0, 9);

	glDisableClientState(GL_VERTEX_ARRAY);
}

void init()
{
	Device* mDevice = GetDevice();

	if (!mDevice->Init(0, NULL))
	{
		return;
	}
}

void frame()
{
	Device* mDevice = GetDevice();

	os::event_loop();

	GetInputManager()->EventLoop();

	GetDevice()->GetRenderer()->_BeginFrame();
	draw_triangle();
	GetDevice()->GetRenderer()->_EndFrame();

	os::swap_buffers();
}

void shutdown()
{
	GetDevice()->Shutdown();
}

JNIEXPORT bool JNICALL Java_crown_android_CrownLib_init(JNIEnv* env, jobject obj)
{
	Log::I("Crown initialized.");
	init();
}

JNIEXPORT bool JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj)
{
	Log::I("Render frame.");
	frame();
}

JNIEXPORT bool JNICALL Java_crown_android_CrownLib_shutdown(JNIEnv* env, jobject obj)
{
	Log::I("Shutdown Crown.");
	shutdown();
}



} // namespace crown
