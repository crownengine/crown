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

class MainScene: public Scene, public KeyboardListener, public MouseListener
{

public:

	MainScene(uint windowWidth, uint windowHeight)
	{
		GetDevice()->GetInputManager()->RegisterMouseListener(this);
		GetDevice()->GetInputManager()->RegisterKeyboardListener(this);
	}

	virtual ~MainScene()
	{
		if (quadric)
			gluDeleteQuadric(quadric);
	}

	virtual void ButtonReleased(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			GLint view[4];
			GLdouble proj[16], model[16];

			glGetDoublev(GL_MODELVIEW_MATRIX, model);
			glGetDoublev(GL_PROJECTION_MATRIX, proj);
			glGetIntegerv(GL_VIEWPORT, view);

			int x = event.x;
			int y = event.y;

			// Adjust y wndCoord
			y = (625 - y);

			double sX, sY, sZ;
			double eX, eY, eZ;

			gluUnProject(x, y, 0.0f, model, proj, view, &sX, &sY, &sZ);
			gluUnProject(x, y, 1.0f, model, proj, view, &eX, &eY, &eZ);

			Vec3 dir = Vec3(eX, eY, eZ) - Vec3(sX, sY, sZ);

			dir.Normalize();

			ray.origin = cam->GetPosition();
			ray.direction = dir;

			//std::cout << x << " " << y << std::endl;
			//std::cout << "Ori: " << ray.origin.ToStr().c_str() << std::endl;
			//std::cout << "Dir: " << ray.direction.ToStr().c_str() << std::endl;
		}
	}

	virtual void KeyPressed(const KeyboardEvent& event)
	{
		if (event.key == KC_UP)
		{
			ray.origin += Vec3(0, 0, -0.1);
		}

		if (event.key == KC_DOWN)
		{
			ray.origin += Vec3(0, 0, 0.1);
		}

		if (event.key == KC_LCONTROL)
		{
			cam->SetActive(!cam->IsActive());
		}
	}
		

	virtual void OnLoad()
	{
		Scene::OnLoad();

		Crown::Renderer* renderer = Crown::GetDevice()->GetRenderer();
		renderer->SetClearColor(Crown::Color4(0.0f, 0.0f, 0.0f, 1.0f));

		// Add a movable camera
		cam = AddMovableCamera();

		if (cam)
		{
			cam->SetActive(true);
			cam->SetSpeed(0.1);
			cam->SetFarClipDistance(1000.0f);
			//cam->SetPosition(Vec3(25, 0, 15));

		}

		grid = GetMeshManager()->LoadGrid("grid", 32);

		ray.origin = Vec3(0, 0, 10);
		ray.direction = Vec3(0, 0, -1);

		sphere[0].c = Vec3(0, 0, 0);
		sphere[0].r = 2.0f;

		sphere[1].c = Vec3(3, -1, -5);
		sphere[1].r = 1.0f;

		sphere[2].c = Vec3(-5, 3, 2);
		sphere[2].r = 3.0f;

		quadric = gluNewQuadric();
	}

	virtual void RenderScene()
	{
		Renderer* renderer = GetDevice()->GetRenderer();

		renderer->_SetLighting(false);
		renderer->_SetBlending(false);
		renderer->_SetAlphaTest(false);
		Scene::RenderScene();

		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		renderer->_SetTexturing(0, false);
		renderer->_SetTexturing(1, false);
		renderer->_SetTexturing(2, false);

		if (cam->IsActive())
		{
			ray.origin = cam->GetPosition();
			ray.direction = cam->GetLookAt();
			//std::cout << ray.direction.ToStr().c_str() << std::endl;
		}

		int sphereIndex = -1;
		real lastDist = 1000000.0f;
		// Check ray-sphere intersection
		for (int i = 0; i < 3; i++)
		{
			real dist;
			if (Intersection::TestRaySphere(ray, sphere[i], dist))
			{
				if (dist <= lastDist)
				{
					lastDist = dist;
					sphereIndex = i;
				}
			}
		}

		for (int i = 0; i < 3; i++)
		{
			if (sphereIndex == i)
			{
				glColor3f(1, 0, 0);
			}
			else
			{
				glColor3f(0, 1, 0);
			}

			tr.LoadIdentity();
			tr.SetTranslation(sphere[i].c);
			renderer->SetMatrix(MT_MODEL, tr);
			gluSphere(quadric, sphere[i].r, 64, 64);
		}

		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		glColor3f(0, 0, 1);
		glBegin(GL_LINES);
		glVertex3fv(ray.origin.ToFloatPtr());
		glVertex3fv((ray.origin + ray.direction).ToFloatPtr());
		glEnd();

		tr.SetTranslation(Vec3(0, -2, 0));
		renderer->SetMatrix(MT_MODEL, tr);
		renderer->RenderMesh(grid.GetPointer());
	}

private:

	MeshPtr grid;
	Mat4 ortho;
	Mat4 crateModel;
	Mat4 tr;

	Ray ray;
	Sphere sphere[3];
	GLUquadric* quadric;
	MovableCamera* cam;
	Mouse* mouse;
	Keyboard* keyboard;
	InputManager* input;
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

	MainScene* mainScene = new MainScene(wndW, wndH);
	GetDevice()->GetSceneManager()->SelectNextScene(mainScene);

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - Intersection Test");

	mDevice->Run();

	mDevice->Shutdown();

	return 0;
}

