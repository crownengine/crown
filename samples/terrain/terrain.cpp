#include "Crown.h"
#include "Terrain.h"
#include <cstdlib>
#include <GL/glew.h>
#include <GL/glu.h>
#include "OS.h"

using namespace crown;

class WndCtrl: public KeyboardListener
{
public:

	WndCtrl()
	{
		GetInputManager()->RegisterKeyboardListener(this);
	}

	void KeyReleased(const KeyboardEvent& event)
	{
		if (event.key == KC_ESCAPE)
		{
			GetDevice()->StopRunning();
		}
	}
};

void DrawCircle(const Vec3& pos, float radius);

class MainScene: public KeyboardListener, public MouseListener
{

public:

	MainScene() :
		optShowSkybox(true),
		optShowCrate(true),
		optShowTerrain(true)
	{
		GetInputManager()->RegisterKeyboardListener(this);
		GetInputManager()->RegisterMouseListener(this);
		mouseRightPressed = false;
		mouseLeftPressed = false;
	}

	~MainScene()
	{
	}

	void KeyReleased(const KeyboardEvent& event)
	{
		if (event.key == '1')
		{
			terrain.PlotCircle(2, 2, 2, 2);
		}

		if (event.key == '2')
		{
			terrain.PlotCircle(4, 4, 4, 2);
		}

		if (event.key == '3')
		{		
			terrain.PlotCircle(8, 8, 8, 2);
		}

		if (event.key == KC_F5)
		{
			terrain.SaveAsBmp("blah.bmp");
		}

		if (event.key == KC_SPACE)
		{
			if (cam)
			{
				cam->SetActive(!cam->IsActive());
			}
		}
	}

	void ButtonPressed(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			mouseLeftPressed = true;

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

			dir.normalize();

			//ray.origin = cam->GetPosition();
			ray.direction = dir;

			//std::cout << x << " " << y << std::endl;
			//std::cout << "Ori: " << ray.origin.ToStr().c_str() << std::endl;
			//std::cout << "Dir: " << ray.direction.ToStr().c_str() << std::endl;
		}
		else if (event.button == MB_RIGHT)
		{
			mouseRightPressed = true;
		}
		wheel += event.wheel * 0.25;
	}

	void ButtonReleased(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			mouseLeftPressed = false;
		}
		else if (event.button == MB_RIGHT)
		{
			mouseRightPressed = false;
		}
		wheel -= event.wheel * 0.25;
	}
		

	void OnLoad()
	{
		crown::Renderer* renderer = crown::GetDevice()->GetRenderer();
		renderer->SetClearColor(Color4::LIGHTBLUE);

		// Add a movable camera
		cam = new MovableCamera(Vec3::ZERO, true, 90.0f, 1.6f, true, 0.1, 2.5);

		if (cam)
		{
			cam->SetActive(true);
			cam->SetSpeed(0.1);
			cam->SetFarClipDistance(1000.0f);
		}

		// Add a skybox
		skybox = new Skybox(Vec3::ZERO, true);

		if (skybox)
		{
			skybox->SetFace(SF_NORTH,	GetTextureManager()->Load("res/red_north.tga"));
			skybox->SetFace(SF_SOUTH,	GetTextureManager()->Load("res/red_south.tga"));
			skybox->SetFace(SF_EAST,	GetTextureManager()->Load("res/red_east.tga"));
			skybox->SetFace(SF_WEST,	GetTextureManager()->Load("res/red_west.tga"));
			skybox->SetFace(SF_UP,		GetTextureManager()->Load("res/red_up.tga"));
			skybox->SetFace(SF_DOWN,	GetTextureManager()->Load("res/red_down.tga"));
		}

		terrain.CreateTerrain(64, 64, 1, 0.0f);

		grass = GetTextureManager()->Load("res/grass.tga");
		grass->SetFilter(TF_TRILINEAR);

		terrain.PlotCircle(4, 4, 4, 2);

		//terrain.ApplyBrush(32, 32, 1.25f);
		terrain.UpdateVertexBuffer(true);
	}

	void RenderScene()
	{
		Renderer* renderer = GetDevice()->GetRenderer();

		cam->Render();

		renderer->_SetLighting(false);
		renderer->_SetTexturing(0, false);

		if (skybox)
		{
			skybox->Render();
		}

		if (cam->IsActive())
		{
			ray.origin = cam->GetPosition();
			ray.direction = cam->GetLookAt();
		}

		/* Render the terrain */
		renderer->_SetAmbientLight(Color4(0.5f, 0.5f, 0.5f, 1.0f));

		renderer->_SetLighting(true);
		renderer->_SetLight(0, true);
		renderer->_SetLightParams(0, LT_DIRECTION, Vec3(0.6, 0.5f, -2.0f));
		renderer->_SetLightColor(0, Color4::WHITE, Color4::WHITE, Color4(0.6f, 0.6f, 0.6f));
		renderer->_SetLightAttenuation(0, 1, 0, 0);

		renderer->_SetMaterialParams(Color4(0.3f, 0.3f, 0.3f), Color4(0.8f, 0.8f, 0.8f), Color4::BLACK, Color4::BLACK, 0);

		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		renderer->_SetTexturing(0, true);
		renderer->SetTexture(0, grass);
		renderer->_SetLighting(true);
		glColor3f(1, 1, 1);
		terrain.Render();

		/* Test for intersection */
		Triangle tri, tri2;
		real dist;
		if (terrain.TraceRay(ray, tri, tri2, dist))
		{
			renderer->_SetDepthTest(false);
			Vec3 intersectionPoint = ray.origin + (ray.direction * dist);
			if (mouseLeftPressed)
			{
				terrain.ApplyBrush(intersectionPoint, 0.09f);
				terrain.UpdateVertexBuffer(true);
			}
			if (mouseRightPressed)
			{
				terrain.ApplyBrush(intersectionPoint, -0.09f);
				terrain.UpdateVertexBuffer(true);
			}
			renderer->_SetDepthTest(true);
		}
	}

private:

	MovableCamera* cam;
	Skybox* skybox;
	Mat4 ortho;
	Terrain terrain;
	Texture* grass;
	Mesh* cube;

	bool optShowSkybox;
	bool optShowCrate;
	bool optShowTerrain;
	bool mouseLeftPressed;
	bool mouseRightPressed;
	float wheel;
	Ray ray;
};

int main(int argc, char** argv)
{
	Device* mDevice = GetDevice();

	if (!mDevice->Init(argc, argv))
	{
		return 0;
	}

	WndCtrl ctrl;
	MainScene mainScene;
	mainScene.OnLoad();

	while (mDevice->IsRunning())
	{
		os::event_loop();

		GetInputManager()->EventLoop();

		GetDevice()->GetRenderer()->_BeginFrame();
			mainScene.RenderScene();
		GetDevice()->GetRenderer()->_EndFrame();

		os::swap_buffers();
	}

	mDevice->Shutdown();

	return 0;
}

