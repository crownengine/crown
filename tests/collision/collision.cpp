#include "Crown.h"
#include <cstdlib>
#include <GL/glu.h>

using namespace Crown;

class WndCtrl: public KeyboardListener
{
public:

	WndCtrl()
	{
		GetDevice()->GetInputManager()->RegisterKeyboardListener(this);
	}

	virtual void KeyReleased(const KeyboardEvent& event)
	{
		if (event.key == KC_ESCAPE)
		{
			GetDevice()->StopRunning();
		}
	}
};

class MainScene: public MouseListener//, public KeyboardListener
{

public:

	MainScene()
	{
		GetDevice()->GetInputManager()->RegisterMouseListener(this);
		//GetDevice()->GetInputManager()->RegisterKeyboardListener(this);

		sphereCount = 0;
	}

	virtual ~MainScene()
	{
		if (quadric)
			gluDeleteQuadric(quadric);
	}

	virtual void ButtonPressed(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			if (sphereCount < 12)
			{
				sphere[sphereCount].c = cam->GetPosition();
				velocity[sphereCount] = cam->GetLookAt() * 0.7f;
				sphereCount++;
			}
		}

		if (event.button == MB_RIGHT)
		{
			sphereCount = 0;
		}
	}

	virtual void OnLoad()
	{
		Crown::Renderer* renderer = Crown::GetDevice()->GetRenderer();
		renderer->SetClearColor(Crown::Color4(0.0f, 0.0f, 0.0f, 1.0f));

		// Add a movable camera
		cam = new MovableCamera(Vec3::ZERO, Angles::ZERO, true, 90.0f, 1.6f, 0.01, 2.5, true);

		if (cam)
		{
			cam->SetActive(true);
			cam->SetSpeed(0.1);
			cam->SetFarClipDistance(1000.0f);
			cam->SetPosition(Vec3(0, 10, 15));

		}

		grass = GetTextureManager()->Load("res/texturewhichdoesnotexists.ctex");

		dplane = GetMeshManager()->LoadPlane("3dplane", 128.0f, 128.0f);

		ray.origin = Vec3(0, 0, 10);
		ray.direction = Vec3(0, 0, -1);

		for (int i = 0; i < 12; i++)
		{
			sphere[i].r = 1.5f;
		}

		plane[0].n			= Vec3(0, 1, 0);
		plane[0].d			= 0;

		plane[1].n 			= Vec3(0, 0, 1);
		plane[1].d			= 0;

		plane[2].n			= Vec3(1, 0, 0);
		plane[2].d			= 0;

		gravity				= Vec3(0, -0.003, 0);

		quadric = gluNewQuadric();
	}

	virtual void RenderScene()
	{
		Renderer* renderer = GetDevice()->GetRenderer();

		renderer->_SetLighting(false);
		renderer->_SetBlending(false);
		renderer->_SetAlphaTest(false);
		renderer->_SetBackfaceCulling(false);
		renderer->_SetTexturing(0, false);
		renderer->_SetTexturing(1, false);
		renderer->_SetTexturing(2, false);

		cam->Render();

		for (int i = 0; i < sphereCount; i++)
		{
			velocity[i] += gravity;
			for (int j = 0; j < 3; j++)
			{
				real itt;
				Vec3 intersectionPoint;
				if (Intersection::TestDynamicSpherePlane(sphere[i], velocity[i], plane[j], itt, intersectionPoint))
				{
					real velLength = velocity[i].GetLength();

					velocity[i].Normalize();
					Vec3 nuovo = ((2.0f * (-velocity[i].Dot(plane[j].n))) * plane[j].n) + velocity[i];
					velocity[i] = nuovo * velLength * 0.7f;
				}
			}
		}

		for (int i = 0; i < sphereCount; i++)
		{
			if (velocity[i] == Vec3::ZERO)
			{
				velocity[i].Zero();
			}
			sphere[i].c += velocity[i];
		}

		if (cam->IsActive())
		{
			ray.origin = cam->GetPosition();
			ray.direction = cam->GetLookAt();
		}

		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);

		for (int j = 0; j < sphereCount; j++)
		{
			// Draw spheres
			glColor3f(1, 0, 0);
			tr.LoadIdentity();
			tr.SetTranslation(sphere[j].c);
			renderer->SetMatrix(MT_MODEL, tr);
			gluSphere(quadric, sphere[j].r, 64, 64);
		}

//		tr.SetTranslation(sphere[1].c);
//		renderer->SetMatrix(MT_MODEL, tr);
//		gluSphere(quadric, sphere[1].r, 64, 64);

//		tr.SetTranslation(sphere[2].c);
//		renderer->SetMatrix(MT_MODEL, tr);
//		gluSphere(quadric, sphere[2].r, 64, 64);
		// End Draw spheres

//		// Draw velocity segments
//		glColor3f(1, 0, 0);
//		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);

//		glBegin(GL_LINES);
//		glVertex3fv(sphere[0].c.ToFloatPtr());
//		glVertex3fv((sphere[0].c + velocity[0]).ToFloatPtr());
//		glEnd();

//		glBegin(GL_LINES);
//		glVertex3fv(sphere[1].c.ToFloatPtr());
//		glVertex3fv((sphere[1].c + velocity[1]).ToFloatPtr());
//		glEnd();

//		glBegin(GL_LINES);
//		glVertex3fv(sphere[2].c.ToFloatPtr());
//		glVertex3fv((sphere[2].c + velocity[2]).ToFloatPtr());
//		glEnd();
//		// End Draw velocity segments

//		// Draw end spheres
//		glColor3f(1.0, 0.0, 0.0);
//		tr.LoadIdentity();
//		tr.SetTranslation(sphere[0].c + velocity[0]);
//		renderer->SetMatrix(MT_MODEL, tr);
//		gluSphere(quadric, sphere[0].r, 64, 64);

//		tr.SetTranslation(sphere[1].c + velocity[1]);
//		renderer->SetMatrix(MT_MODEL, tr);
//		gluSphere(quadric, sphere[1].r, 64, 64);

//		tr.SetTranslation(sphere[2].c + velocity[2]);
//		renderer->SetMatrix(MT_MODEL, tr);
//		gluSphere(quadric, sphere[2].r, 64, 64);
//		// End Draw end spheres

//		// Draw spheres at collision time
//		glColor3f(0.0, 1.0, 0.0);
//		real it;
//		if (Intersection::TestDynamicSphereSphere(sphere[0], velocity[0], sphere[1], velocity[1], it))
//		{
//			tr.LoadIdentity();

//			tr.SetTranslation(sphere[0].c + (velocity[0] * it));
//			renderer->SetMatrix(MT_MODEL, tr);
//			gluSphere(quadric, sphere[0].r, 64, 64);


//			tr.SetTranslation(sphere[1].c + (velocity[1] * it));
//			renderer->SetMatrix(MT_MODEL, tr);
//			gluSphere(quadric, sphere[1].r, 64, 64);
//		}

//		if (Intersection::TestDynamicSpherePlane(sphere[2], velocity[2], plane, it))
//		{
//			tr.LoadIdentity();

//			tr.SetTranslation(sphere[2].c + (velocity[2] * it));
//			renderer->SetMatrix(MT_MODEL, tr);
//			gluSphere(quadric, sphere[2].r, 64, 64);
//		}
//		// End Draw spheres at collision time

		renderer->_SetTexture(0, grass);
		renderer->_SetTexturing(0, true);
		renderer->_SetTextureMode(0, grass->GetMode(), grass->GetBlendColor());
		renderer->_SetTextureFilter(0, grass->GetFilter());

		tr.BuildRotationX(Math::PI / 2.0f);
		tr.SetTranslation(Vec3(0, 0, 0));
		renderer->SetMatrix(MT_MODEL, tr);
		renderer->RenderVertexIndexBuffer(dplane->GetVertexBuffer(), dplane->GetIndexBuffer());

		tr.LoadIdentity();
		tr.SetTranslation(Vec3(0, 0, 0));
		renderer->SetMatrix(MT_MODEL, tr);
		renderer->RenderVertexIndexBuffer(dplane->GetVertexBuffer(), dplane->GetIndexBuffer());

		tr.BuildRotationY(Math::PI / 2.0f);
		tr.SetTranslation(Vec3(0, 0, 0));
		renderer->SetMatrix(MT_MODEL, tr);
		renderer->RenderVertexIndexBuffer(dplane->GetVertexBuffer(), dplane->GetIndexBuffer());
	}

private:

	Mesh* dplane;
	Mat4 ortho;
	Mat4 crateModel;
	Mat4 tr;
	Texture* grass;

	Ray ray;
	Plane plane[3];
	Sphere sphere[12];
	Vec3 velocity[12];
	Vec3 gravity;
	GLUquadric* quadric;
	MovableCamera* cam;
	int sphereCount;
};

int main(int argc, char** argv)
{
	int wndW = 1000;
	int wndH = 625;

	if (argc == 3)
	{
		wndW = atoi(argv[1]);
		wndH = atoi(argv[2]);
	}

	Device* mDevice = GetDevice();

	if (!mDevice->Init(wndW, wndH, 32, false))
	{
		return 0;
	}

	WndCtrl ctrl;

	MainScene scene;
	scene.OnLoad();

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - Collision Test");

	while (mDevice->IsRunning())
	{
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

		scene.RenderScene();

		GetDevice()->GetRenderer()->_EndFrame();

		mDevice->GetMainWindow()->Update();
	}

	mDevice->Shutdown();

	mDevice->Shutdown();

	return 0;
}

