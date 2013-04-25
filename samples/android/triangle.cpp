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
		cam->MoveBackward();
	}

	void touch_move(const TouchEvent& event)
	{
		cam->MoveBackward();
	}

	void draw_triangle()
	{
		GetDevice()->renderer()->set_clear_color(Color4::LIGHTBLUE);

		GetDevice()->renderer()->set_matrix(MT_VIEW, Mat4::IDENTITY);
		GetDevice()->renderer()->set_matrix(MT_MODEL, Mat4::IDENTITY);

		cam->Render();

		static GLfloat vertices[] = {  -1.0f, -1.0f, -2.0f,
										1.0f, -1.0f, -2.0f,
										0.0f, 1.0f, -2.0f};

		GetDevice()->renderer()->set_matrix(MT_MODEL, Mat4::IDENTITY);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glDisableClientState(GL_VERTEX_ARRAY);
	}

	void frame()
	{
		Device* engine = GetDevice();

		os::event_loop();

		get_input_manager()->event_loop();

		engine->renderer()->begin_frame();
		draw_triangle();
		engine->renderer()->end_frame();
	}
};

MainScene* scene = new MainScene();

JNIEXPORT void JNICALL Java_crown_android_CrownLib_frame(JNIEnv* env, jobject obj)
{
	scene->frame();
}

} // namespace crown
