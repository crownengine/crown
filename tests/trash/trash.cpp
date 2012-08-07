#include "EventBuffer.h"
#include <cstdio>
#include <cstring>
#include "System.h"
#include "Crown.h"
#include <GL/glew.h>

using namespace Crown;

class WndCtrl: public KeyboardListener
{
public:

	WndCtrl()
	{
		GetDevice()->GetInputManager()->RegisterKeyboardListener(this);
	}

	void KeyReleased(const KeyboardEvent& event)
	{
		if (event.key == KC_ESCAPE)
		{
			GetDevice()->StopRunning();
		}
	}
};

int main(int argc, char** argv)
{
	if (!GetDevice()->Init(argc, argv))
	{
		return -1;
	}

	GetDevice()->GetMainWindow()->SetTitle("System test");

	WndCtrl ctrl;

	//-------------- WORLD -----------------------------

	Triangle tri(Vec3(-1.5, -1.5, 2), Vec3(1.5, -1.5, 2), Vec3(0, 1.5, 2));

	Plane triPlane = tri.ToPlane();

	Sphere s;
	s.c = Vec3(0, 0, 5);
	s.r = 0.05f;

	GLUquadric* quadric = gluNewQuadric();

	//-------------- WORLD -----------------------------

	MovableCamera cam(Vec3::ZERO, Angles::ZERO, true, 90.0f, 1.6f, true, 0.1, 2.5);

	Device* mDevice = GetDevice();

	Timer timer;

	Crown::ulong dt = 0;
	Crown::ulong sTime = 0;
	Crown::ulong eTime = 0;
	while (mDevice->IsRunning())
	{
		dt = eTime - sTime;
		sTime = timer.GetMilliseconds();

		InputManager* mInputManager = GetDevice()->GetInputManager();

		if (mInputManager)
		{
			if (mInputManager->IsMouseAvailable())
			{
				if (mInputManager->IsMouseAvailable())
				{
					mInputManager->GetMouse()->EventLoop();
				}
				if (mInputManager->IsKeyboardAvailable())
				{
					mInputManager->GetKeyboard()->EventLoop();
				}
				if (mInputManager->IsTouchAvailable())
				{
					mInputManager->GetTouch()->EventLoop();
				}
			}
		}

		mDevice->GetMainWindow()->EventLoop();

		GetDevice()->GetRenderer()->_BeginFrame();

		GetDevice()->GetRenderer()->SetClearColor(Color4::GREY);
		GetDevice()->GetRenderer()->_SetLighting(false);
		GetDevice()->GetRenderer()->_SetTexturing(0, false);
		GetDevice()->GetRenderer()->_SetBackfaceCulling(false);

		cam.Render();

		GetDevice()->GetRenderer()->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		glColor3f(0, 1, 0);
		glBegin(GL_TRIANGLES);
			glVertex3fv(tri.v1.ToFloatPtr());
			glVertex3fv(tri.v2.ToFloatPtr());
			glVertex3fv(tri.v3.ToFloatPtr());
		glEnd();

		Vec3 sphereVelocity = cam.GetLookAt() * 15.0f;
		Vec3 sphereCenter = cam.GetPosition();

		s.c = sphereCenter;

		real it;
		Vec3 intersectionPoint;
		if (Intersection::TestDynamicSphereTriangle(s, sphereVelocity, tri, it, intersectionPoint))
		{
			Mat4 sphereTranslation = Mat4::IDENTITY;
			sphereTranslation.SetTranslation(s.c + (sphereVelocity * it));
			GetDevice()->GetRenderer()->SetMatrix(MT_MODEL, sphereTranslation);
			glColor3f(1, 0, 0);
			gluSphere(quadric, s.r, 64, 64);


			Renderer* renderer = GetDevice()->GetRenderer();
			renderer->_SetDepthTest(false);
			renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
			glColor3f(0, 0, 1);
			renderer->_SetPointSize(3.0f);
			glBegin(GL_POINTS);
				glVertex3fv(intersectionPoint.ToFloatPtr());
			glEnd();
			renderer->_SetPointSize(1.0f);
			renderer->_SetDepthTest(true);
		}

		GetDevice()->GetRenderer()->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		glBegin(GL_LINES);
			glVertex3fv(s.c.ToFloatPtr());
			glVertex3fv((s.c + sphereVelocity).ToFloatPtr());
		glEnd();

		GetDevice()->GetRenderer()->_EndFrame();

		mDevice->GetMainWindow()->Update();

		eTime = timer.GetMilliseconds();
	}

	mDevice->Shutdown();
}

