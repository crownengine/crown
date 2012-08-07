#include "Crown.h"
#include <time.h>
#include <cstdlib>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "WorldTerrain.h"
#include "perlin.h"

void DrawAxis();
void DrawGrid(float xStart, float zStart, float xStep, float zStep);
void DrawCube(float x, float y, float z);
void DrawVoxelGrid(float xStart, float yStart, float zStart, float xStep, float yStep, float zStep);

using namespace Crown;

MovableCamera* cam;
WorldTerrain* terrain;
int groundLevel = 0;
int seed;
int octaves = 4, freq = 2;
int currentFog = 10;
float fogDensity = 0.1f;

bool attached = true;

class MainScene: public Crown::Scene, EventReceiver {

public:

	MainScene()
	{
		GetDevice()->GetEventDispatcher()->RegisterEventReceiver(this);
		mDevice = GetDevice();
	}

	virtual ~MainScene()
	{
		if (terrain)
			delete terrain;
	}

	void HandleEvent(const Event& event)
	{
		if (event.event_type == ET_KEYBOARD)
		{
			if (event.keyboard.type == KET_RELEASED)
			{
				if (event.keyboard.key == KK_ESCAPE)
				{

					if (mDevice) {
						mDevice->StopRunning();
					}
				}

				if (event.keyboard.key == KK_SPACE) {
					
					seed = rand();
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_UP) {
					
					groundLevel += 1;
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_DOWN) {
					
					groundLevel -= 1;
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_R) {
					
					octaves += 1;
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_F) {
					
					if (octaves > 1)
						octaves -= 1;
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_T) {
					
					freq += 1;
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_G) {
					
					if (freq > 1)
						freq -= 1;
					terrain->RandomMap(groundLevel, seed, octaves, freq);
				}

				if (event.keyboard.key == KK_Y) {
					

					currentFog+=10;
					glFogf(GL_FOG_END, currentFog);
				}

				if (event.keyboard.key == KK_H) {
					
					currentFog-=10;
					glFogf(GL_FOG_END, currentFog);
				}

				if (event.keyboard.key == KK_1) {
					
					terrain->ToggleGrid();
				}

				if (event.keyboard.key == KK_2) {
					
					terrain->CycleViewDistance();
				}

				if (event.keyboard.key == KK_3) {
					
					static bool fogActive = true;
					if (fogActive)
						glDisable(GL_FOG);
					else
						glEnable(GL_FOG);
					fogActive = ! fogActive;
				}

				if (event.keyboard.key == KK_0) {
					
					attached = !attached;
				}

			}

			if (event.keyboard.key == KK_Q)
			{
				terrain->LOL();
			}
		}
	}

	virtual void OnLoad() {

		SceneNode* sn = new SceneNode(this, NULL, Vec3(), Angles(), true);

		cam = AddMovableCamera(
		sn,
		Vec3((CHUNKS_COUNT+2)/2.0f*CHUNK_SIZE*CubeSize, CHUNKS_COUNT_H*0.7*CHUNK_SIZE*CubeSize, (CHUNKS_COUNT+2)/2.0f*CHUNK_SIZE*CubeSize),
		//Vec3(0, 1.5*CHUNK_SIZE*CubeSize, 0),
		Angles(0, 0, 0),
		true,
		90.0f,
		1.59f,
		true,
		1.5,
		1);
		cam->SetActive(true);
		cam->SetSpeed(0.1);
		cam->SetFarClipDistance(150.0f);

		GetDevice()->GetRenderer()->SetClearColor(Color(0.457f, 0.754f, 1.0f, 1.0f));

		srand(time(NULL));
		seed = rand();
		terrain = new WorldTerrain();
		terrain->RandomMap(groundLevel, seed, octaves, freq);
		
		timer = Crown::Timer::GetInstance();
		beforeTime = timer->GetMilliseconds();
	}

	virtual void RenderScene() {

		Scene::RenderScene();

		cam->Render();

		Mat4 identity;
		identity.LoadIdentity();

		GetDevice()->GetRenderer()->SetMatrix(MT_MODEL, identity);

		DrawAxis();

		if (attached)
			terrain->SetPlayerPosition(cam->GetPosition());

		terrain->Render();

		if (timer->GetMilliseconds() - beforeTime > 1000)
		{
			Renderer* renderer = mDevice->GetRenderer();
			Str title = "Fps: " + Str(renderer->GetFPS()) + " - Regenerated Chunks: " + Str(terrain->RegeneratedChunks) + " - Px: " + Str(terrain->GetPlayerPosition().x) + ", Px: " + Str(terrain->GetPlayerPosition().z);
			terrain->RegeneratedChunks = 0;
			GetDevice()->GetMainWindow()->SetTitle(title.c_str());
			beforeTime = timer->GetMilliseconds();
		}
	}

	Device* mDevice;
	Crown::Timer* timer;
	Crown::ulong beforeTime;
};


int main(int argc, char** argv) {

	int wndW, wndH;
	wndW = 1024;
	wndH = 768;
	bool full = false;

	if (argc == 3) {
		wndW = atoi(argv[1]);
		wndH = atoi(argv[2]);
	}

	Device* device = GetDevice();


	if (!device->Init(wndW, wndH, 32, false)) {

		return 0;
	}

	Renderer* renderer = device->GetRenderer();
	SceneManager* smgr = device->GetSceneManager();
	ResourceManager* resman = device->GetResourceManager();

	device->GetMainWindow()->SetTitle("Crown Engine v0.1");
	device->GetMainWindow()->GetCursorControl()->SetVisible(true);
	device->GetMainWindow()->SetFullscreen(full);

	Scene* scene = new MainScene();
	
	smgr->SelectNextScene(scene);

	device->Run();

	device->Shutdown();

//	Scene* scene = new Scene();
//
//	cam = scene->AddMovableCamera(
//		0,
//		Vec3((CHUNKS_COUNT+2)/2.0f*CHUNK_SIZE*CubeSize, CHUNKS_COUNT_H*0.7*CHUNK_SIZE*CubeSize, (CHUNKS_COUNT+2)/2.0f*CHUNK_SIZE*CubeSize),
//		//Vec3(0, 1.5*CHUNK_SIZE*CubeSize, 0),
//		Angles(0, 0, 0),
//		Vec3(1, 1, 1),
//		true,
//		90.0f,
//		1.59f,
//		true,
//		1.5,
//		1);
//	cam->SetActive(true);
//	cam->SetSpeed(0.1);
//	cam->SetFarClipDistance(150.0f);
//
//	std::cout << "Entity count: " << scene->GetEntityCount() << std::endl;
//	std::cout << "Light count: " << scene->GetLightCount() << std::endl;
//	std::cout << "Sizeof RenderWindow: " << sizeof(RenderWindow) << std::endl;
///*
//	Material material;
//	material.mAmbient = Color(0.1, 0.1, 0.1, 1);
//	material.mDiffuse = Color(1, 1, 1, 1);
//	material.mSpecular = Color(0, 0, 0, 1);
//	material.mShininess = 128;
//	material.mSeparateSpecularColor = true;
//	material.mTexturing = false;
//	material.mLighting = true;
//
//	renderer->SetMaterial(material);
//*/
//	Mat4 identity;
//	identity.LoadIdentity();
//
//	renderer->SetClearColor(Color(0.457f, 0.754f, 1.0f, 1.0f));
//
//	Mat4 ortho;
//	ortho.BuildProjectionOrthoRH(wndW, wndH, 1, -1);
//
//	Mat4 perspective;
//	perspective.BuildProjectionPerspectiveRH(90.0f, 1.59f, 0.1f, 100.0f);
//
//	Mat4 text;
//	text.LoadIdentity();
//	text.SetTranslation(Vec3(400, 350, 0));
//
//
//	//Crown::Font font;
//	//Image* testImg = font.LoadFont("tests/font/arialbd.ttf");
//
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//	srand(time(NULL));
//	seed = rand();
//	terrain = new WorldTerrain();
//	terrain->RandomMap(groundLevel, seed, octaves, freq);
//	//terrain->RandomMap1();
//
//	while (device->IsRunning()) {
//
//		WindowEventHandler::GetInstance()->ManageEvents();
//		GetDevice()->GetMainWindow()->GetRenderContext()->MakeCurrent();
//
//		// ----------- Begin Scene -----------
//		renderer->_BeginFrame();
//
//		renderer->SetMatrix(MT_PROJECTION, ortho);
//		//renderer->SetMatrix(MT_MODEL, text);
//
//		glDisable(GL_LIGHTING);
//		glColor3f(1, 1, 1);
//
//		cam->Render();
//
//		renderer->SetMatrix(MT_MODEL, identity);
//
//		DrawAxis();
//
//		if (attached)
//			terrain->SetPlayerPosition(cam->GetPosition());
//
//		terrain->Render();
//
//		renderer->_EndFrame();
//		// ----------- End Scene -------------
//
//		GetDevice()->GetMainWindow()->Update();
//		GetDevice()->GetMainWindow()->SetTitle(Str(renderer->GetFPS()).c_str());
//	}
//
//	delete terrain;
//
//	device->Shutdown();

	return 0;
}

void DrawAxis() {

	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(1000, 0, 0);
	glColor3f(0, 1, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 1000, 0);
	glColor3f(0, 0, 1);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 1000);
	glEnd();
}

void DrawGrid(float xStart, float zStart, float xStep, float zStep) {

	glColor3f(.5f, .5f, .5f);
	const unsigned int steps = 128;
	for (unsigned int i = 0; i <= steps; i++) {
		glBegin(GL_LINES);
		glVertex3f(xStart + xStep * (float)i, -1.0f, zStart);
		glVertex3f(xStart + xStep * (float)i, -1.0f, zStart-steps*zStep);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(xStart, -1.0f, zStart - zStep * (float)i);
		glVertex3f(xStart+steps*xStep, -1.0f, zStart - zStep * (float)i);
		glEnd();
	}
}

void DrawCube(float x, float y, float z) {

	glBegin(GL_QUADS);

	
	//top
	glColor3f(.3f, .0f, .0f);
	glVertex3f(x           , y + CubeSize, z);
	glColor3f(.25f, .0f, .0f);
	glVertex3f(x + CubeSize, y + CubeSize, z);
	glColor3f(.3f, .0f, .0f);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);
	glColor3f(.25f, .0f, .0f);
	glVertex3f(x           , y + CubeSize, z - CubeSize);

	//bottom
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x           , y, z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x           , y, z - CubeSize);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y, z);

	//left
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x, y           , z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x, y + CubeSize, z);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x, y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x, y           , z - CubeSize);

	//right
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y           , z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y           , z - CubeSize);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y + CubeSize, z);

	//front
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x           , y           , z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y           , z);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y + CubeSize, z);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x           , y + CubeSize, z);

	//back
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x           , y           , z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x           , y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .3f);
	glVertex3f(x + CubeSize, y + CubeSize, z - CubeSize);
	glColor3f(.0f, .0f, .25f);
	glVertex3f(x + CubeSize, y           , z - CubeSize);

	glEnd();
}

void DrawVoxelGrid(float xStart, float yStart, float zStart, float xStep, float yStep, float zStep) {

	glColor3f(.5f, .5f, .5f);

	const unsigned int steps = 16;
	glBegin(GL_LINES);
	for (unsigned int i = 0; i <= steps; i++) {
		
		for(unsigned int j = 0; j <= steps; j++) {
			glVertex3f(xStart + xStep * (float)i, j * yStep, zStart);
			glVertex3f(xStart + xStep * (float)i, j * yStep, zStart-steps*zStep);

			glVertex3f(xStart + xStep * (float)j, yStart, zStart - zStep * (float)i);
			glVertex3f(xStart + xStep * (float)j, yStart+steps*yStep, zStart - zStep * (float)i);

			glVertex3f(xStart, j * yStep, zStart - zStep * (float)i);
			glVertex3f(xStart+steps*xStep, j * yStep, zStart - zStep * (float)i);
		}
		
	}
	glEnd();
}

