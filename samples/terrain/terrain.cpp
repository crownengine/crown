#include "Crown.h"
#include "Terrain.h"
#include "FPSSystem.h"
#include "Game.h"

using namespace crown;

class WndCtrl: public KeyboardListener
{
public:

	WndCtrl()
	{
		device()->input_manager()->register_keyboard_listener(this);
	}

	void key_released(const KeyboardEvent& event)
	{
		if (event.key == KC_ESCAPE)
		{
			device()->stop();
		}
	}
};

class MainScene: public KeyboardListener, public MouseListener
{

public:

	MainScene() :
		optShowSkybox(true),
		optShowCrate(true),
		optShowTerrain(true)
	{
		device()->input_manager()->register_keyboard_listener(this);
		device()->input_manager()->register_mouse_listener(this);
		mouseRightPressed = false;
		mouseLeftPressed = false;
	}

	~MainScene()
	{
	}

	void key_released(const KeyboardEvent& event)
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
	}

	void button_pressed(const MouseEvent& event)
	{
		if (event.button == MB_LEFT)
		{
			mouseLeftPressed = true;

			//GLint view[4];
			//GLdouble proj[16], model[16];

			//glGetDoublev(GL_MODELVIEW_MATRIX, model);
			//glGetDoublev(GL_PROJECTION_MATRIX, proj);
			//glGetIntegerv(GL_VIEWPORT, view);

			//int x = event.x;
			//int y = event.y;

			// Adjust y wndCoord
			//y = (625 - y);

			//double sX, sY, sZ;
			//double eX, eY, eZ;

			//gluUnProject(x, y, 0.0f, model, proj, view, &sX, &sY, &sZ);
			//gluUnProject(x, y, 1.0f, model, proj, view, &eX, &eY, &eZ);

			//Vec3 dir = Vec3(eX, eY, eZ) - Vec3(sX, sY, sZ);

			//dir.normalize();

			//ray.direction = dir;
		}
		else if (event.button == MB_RIGHT)
		{
			mouseRightPressed = true;
		}
		wheel += event.wheel * 0.25;
	}

	void button_released(const MouseEvent& event)
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
		
	void on_load()
	{
		crown::Renderer* renderer = crown::device()->renderer();

		renderer->set_clear_color(Color4::LIGHTBLUE);
		
		Vec3 start = Vec3(0.0f, 10.0f, 0.0f);

		// Add a movable camera
		cam = new Camera(start, 90.0f, 1.6f);

		system = new FPSSystem(cam, 10.0f, 2.5f);

		// Add a skybox
		skybox = new Skybox(Vec3::ZERO, true);

		//if (skybox)
		//{
		//	skybox->SetFace(SF_NORTH,	GetTextureManager()->Load("res/red_north.tga"));
		//	skybox->SetFace(SF_SOUTH,	GetTextureManager()->Load("res/red_south.tga"));
		//	skybox->SetFace(SF_EAST,	GetTextureManager()->Load("res/red_east.tga"));
		//	skybox->SetFace(SF_WEST,	GetTextureManager()->Load("res/red_west.tga"));
		//	skybox->SetFace(SF_UP,		GetTextureManager()->Load("res/red_up.tga"));
		//	skybox->SetFace(SF_DOWN,	GetTextureManager()->Load("res/red_down.tga"));
		//}

		terrain.CreateTerrain(64, 64, 1, 0.0f);

		red_north = device()->resource_manager()->load("textures/red_north.tga");
		red_south = device()->resource_manager()->load("textures/red_south.tga");
		red_east  = device()->resource_manager()->load("textures/red_east.tga");
		red_west  = device()->resource_manager()->load("textures/red_west.tga");
		red_up    = device()->resource_manager()->load("textures/red_up.tga");
		red_down  = device()->resource_manager()->load("textures/red_down.tga");

		grass = device()->resource_manager()->load("textures/grass.tga");

		terrain.PlotCircle(4, 4, 4, 2);

		//terrain.ApplyBrush(32, 32, 1.25f);
		terrain.UpdateVertexBuffer(true);
	}

	void on_unload()
	{
		device()->resource_manager()->unload(grass);
		device()->resource_manager()->unload(red_north);
		device()->resource_manager()->unload(red_south);
		device()->resource_manager()->unload(red_east);
		device()->resource_manager()->unload(red_west);
		device()->resource_manager()->unload(red_up);
		device()->resource_manager()->unload(red_down);
	}

	void render(float dt)
	{
		Renderer* renderer = device()->renderer();
		
		system->set_view_by_cursor();
		system->update(dt);

		renderer->set_lighting(false);
		renderer->set_texturing(0, false);

		if (skybox)
		{
			skybox->Render();
		}

		ray.set_origin(cam->position());
		ray.set_direction(cam->look_at());

		/* Render the terrain */
		renderer->set_ambient_light(Color4(0.5f, 0.5f, 0.5f, 1.0f));

		renderer->set_lighting(true);
		renderer->set_light(0, true);
		renderer->set_light_params(0, LT_DIRECTION, Vec3(0.6, 0.5f, -2.0f));
		renderer->set_light_color(0, Color4::WHITE, Color4::WHITE, Color4(0.6f, 0.6f, 0.6f));
		renderer->set_light_attenuation(0, 1, 0, 0);

		renderer->set_material_params(Color4(0.3f, 0.3f, 0.3f), Color4(0.8f, 0.8f, 0.8f), Color4::BLACK, Color4::BLACK, 0);

		renderer->set_matrix(MT_MODEL, Mat4::IDENTITY);

		if (device()->resource_manager()->is_loaded(grass))
		{
			TextureResource* grass_tex = (TextureResource*)device()->resource_manager()->data(grass);
			if (grass_tex)
			{
				TextureId grass_id = grass_tex->m_render_texture;
				renderer->set_texturing(0, true);
				renderer->set_texture(0, grass_id);
				renderer->set_lighting(true);
			}
		}

		
		//glColor3f(1, 1, 1);

		terrain.Render();

		/* Test for intersection */
		Triangle tri, tri2;
		real dist;
		if (terrain.TraceRay(ray, tri, tri2, dist))
		{
			renderer->set_depth_test(false);
			Vec3 intersectionPoint = ray.origin() + (ray.direction() * dist);
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
			renderer->set_depth_test(true);
		}
	}

private:

	FPSSystem* system;
	Camera* cam;
	Skybox* skybox;
	Mat4 ortho;
	Terrain terrain;

	// Resources
	ResourceId grass;
	ResourceId red_north;
	ResourceId red_south;
	ResourceId red_east;
	ResourceId red_west;
	ResourceId red_up;
	ResourceId red_down;

	bool optShowSkybox;
	bool optShowCrate;
	bool optShowTerrain;
	bool mouseLeftPressed;
	bool mouseRightPressed;
	float wheel;
	Ray ray;
};

class TerrainGame : public Game
{
public:

	void init()
	{
		m_scene.on_load();
	}

	void shutdown()
	{
		m_scene.on_unload();
	}

	void update(float dt)
	{
		m_scene.render(dt);
	}

private:

	MainScene m_scene;
	WndCtrl m_ctrl;
};

extern "C" Game* create_game()
{
	return new TerrainGame;
}

extern "C" void destroy_game(Game* game)
{
	delete game;
}

