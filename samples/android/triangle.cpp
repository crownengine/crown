#include "Crown.h"
#include "OS.h"
#include <jni.h>
#include <GLES/gl.h>

namespace crown
{

extern "C"
{
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj);
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

JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj)
{
	Log::I("Render frame.");
	frame();
}

} // namespace crown
