#include "Crown.h"
#include "OS.h"
#include <jni.h>
#include <GLES/gl.h>
#include "MovableCamera.h"

namespace crown
{

MovableCamera* cam;

extern "C"
{
	JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj);
};

class MainScene : public AccelerometerListener, TouchListener
{
	
public:

	MainScene()
	{
		get_input_manager()->register_accelerometer_listener(this);
		get_input_manager()->register_touch_listener(this);

		cam = new MovableCamera(Vec3::ZERO, true, 90.0f, 1.6f, true, 0.1f, 2.5f);

		if (cam)
		{
			cam->SetActive(true);
			cam->SetSpeed(0.1);
			cam->SetFarClipDistance(1000.0f);
		}
	}

	void accelerometer_changed(const AccelerometerEvent& event)
	{
		cam->SetRotation(event.x, event.y);
	}

	void touch_down(const TouchEvent& event)
	{
		cam->MoveForward();
	}

	void touch_move(const TouchEvent& event)
	{
		cam->MoveForward();
	}

	void draw_triangle()
	{
		uint32_t width;
		uint32_t height;
		os::get_render_window_metrics(width, height);

		cam->Render();

		static GLfloat vertices[] = {  -1.0f, -1.0f, -2.0f,
										1.0f, -1.0f, -2.0f,
										0.0f, 1.0f, -2.0f};

		GetDevice()->GetRenderer()->SetMatrix(MT_MODEL, Mat4::IDENTITY);

		Mat4 projection;
		projection.build_projection_perspective_rh(90.0f, (float) width/height, 0.1f, 100.0f);
		GetDevice()->GetRenderer()->SetMatrix(MT_PROJECTION, projection);

		GetDevice()->GetRenderer()->SetClearColor(Color4::LIGHTBLUE);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);

		glDrawArrays(GL_TRIANGLES, 0, 9);

		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void frame()
	{
		Device* mDevice = GetDevice();

		os::event_loop();

		get_input_manager()->event_loop();

		GetDevice()->GetRenderer()->_BeginFrame();
		draw_triangle();
		GetDevice()->GetRenderer()->_EndFrame();

		os::swap_buffers();
	}
};

MainScene* scene = new MainScene();



JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj)
{
	scene->frame();
}

} // namespace crown
