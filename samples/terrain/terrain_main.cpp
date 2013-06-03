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

class MainScene
{

public:

	MainScene() :
		optShowSkybox(true),
		optShowCrate(true),
		optShowTerrain(true),
		camera_active(true)
	{
		mouseRightPressed = false;
		mouseLeftPressed = false;
	}

	~MainScene()
	{
	}

	void poll_input()
	{
		Keyboard* keyb = device()->keyboard();
		Mouse* mouse = device()->mouse();

		if (keyb->key_pressed(KC_1))
		{
			terrain.PlotCircle(2, 2, 2, 2);
		}

		if (keyb->key_pressed(KC_2))
		{
			terrain.PlotCircle(4, 4, 4, 2);
		}

		if (keyb->key_pressed(KC_3))
		{		
			terrain.PlotCircle(8, 8, 8, 2);
		}

		if (keyb->key_pressed(KC_F5))
		{
			device()->reload(grass);
		}

		if (keyb->key_pressed(KC_SPACE))
		{
			camera_active = !camera_active;
		}

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

		mouseLeftPressed = mouse->button_pressed(MB_LEFT);
		mouseRightPressed = mouse->button_pressed(MB_RIGHT);
	}
		
	void on_load()
	{
		crown::Renderer* renderer = crown::device()->renderer();
		
		Vec3 start = Vec3(0.0f, 10.0f, 0.0f);

		// Add a movable camera
		cam = CE_NEW(m_allocator, Camera)(start, 90.0f, 1.6f);
		system = CE_NEW(m_allocator, FPSSystem)(cam, 10.0f, 2.5f);
		// Add a skybox
		skybox = CE_NEW(m_allocator, Skybox)(Vec3::ZERO, true);

		terrain.CreateTerrain(64, 64, 1, 0.0f);
		terrain.PlotCircle(4, 4, 4, 2);
		terrain.UpdateVertexBuffer(true);

		red_north = device()->load("textures/red_north.tga");
		red_south = device()->load("textures/red_south.tga");
		red_east  = device()->load("textures/red_east.tga");
		red_west  = device()->load("textures/red_west.tga");
		red_up    = device()->load("textures/red_up.tga");
		red_down  = device()->load("textures/red_down.tga");
		grass     = device()->load("textures/grass.tga");

		device()->resource_manager()->flush();

		TextureResource* grass_texture = (TextureResource*)device()->data(grass);
		grass_id = device()->renderer()->create_texture(grass_texture->width(), grass_texture->height(), grass_texture->format(), grass_texture->data());

		//rb_id = device()->renderer()->create_render_buffer(200, 200, PF_RGBA_8);
	}

	void on_unload()
	{
		device()->unload(grass);
		device()->unload(red_north);
		device()->unload(red_south);
		device()->unload(red_east);
		device()->unload(red_west);
		device()->unload(red_up);
		device()->unload(red_down);

		CE_DELETE(m_allocator, skybox);
		CE_DELETE(m_allocator, system);
		CE_DELETE(m_allocator, cam);
	}

	void update(float dt)
	{
		poll_input();

		Renderer* renderer = device()->renderer();

		renderer->set_clear_color(Color4::LIGHTBLUE);
		
		if (camera_active)
		{
			system->set_view_by_cursor();
		}
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

		if (device()->is_loaded(grass))
		{
			renderer->set_texturing(0, true);
			renderer->bind_texture(0, grass_id);
		}
		
		//glColor3f(1, 1, 1);

		terrain.Render();

		/* Test for intersection */
		Triangle tri, tri2;
		float dist;
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

	MallocAllocator m_allocator;
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
	TextureId grass_id;
	RenderBufferId rb_id;

	bool optShowSkybox;
	bool optShowCrate;
	bool optShowTerrain;
	bool mouseLeftPressed;
	bool mouseRightPressed;
	float wheel;
	bool camera_active;
	Ray ray;
};

MainScene m_scene;
WndCtrl m_ctrl;

extern "C"
{
	void init()
	{
		m_scene.on_load();
	}

	void shutdown()
	{
		m_scene.on_unload();
	}

	void frame(float dt)
	{
		m_scene.update(dt);
	}
}
