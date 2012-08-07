#include "Crown.h"
#include <cstdlib>

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

class MainScene: public Scene, public KeyboardListener
{

public:

	MainScene() :
		optShowSkybox(true),
		optShowCrate(true),
		optShowTerrain(true)
	{
		GetDevice()->GetInputManager()->RegisterKeyboardListener(this);
	}

	virtual ~MainScene()
	{
	}

	virtual void KeyReleased(const KeyboardEvent& event)
	{
		if (event.key == '1')
		{
			optShowSkybox = !optShowSkybox;
		}

		if (event.key == '2')
		{
			optShowCrate = !optShowCrate;
		}

		if (event.key == '3')
		{
			optShowTerrain = !optShowTerrain;
		}

		if (event.key == KC_F5)
		{
			GetTextureManager()->Unload("res/grass.tga", false);
		}

		if (event.key == KC_SPACE)
		{
			if (cam)
			{
				cam->SetActive(!cam->IsActive());
			}
		}
	}
		

	virtual void OnLoad()
	{
		Crown::Renderer* renderer = Crown::GetDevice()->GetRenderer();
		renderer->SetClearColor(Crown::Color4(0.0f, 0.0f, 0.0f, 1.0f));

		// Add a movable camera
		cam = AddMovableCamera();

		if (cam)
		{
			cam->SetActive(true);
			cam->SetSpeed(0.1);
			cam->SetFarClipDistance(1000.0f);
		}

		// Add a skybox
		skybox = AddSkybox();

		if (skybox)
		{
			skybox->SetFace(SF_NORTH,	GetTextureManager()->Load("res/red_north.tga"));
			skybox->SetFace(SF_SOUTH,	GetTextureManager()->Load("res/red_south.tga"));
			skybox->SetFace(SF_EAST,	GetTextureManager()->Load("res/red_east.tga"));
			skybox->SetFace(SF_WEST,	GetTextureManager()->Load("res/red_west.tga"));
			skybox->SetFace(SF_UP,		GetTextureManager()->Load("res/red_up.tga"));
			skybox->SetFace(SF_DOWN,	GetTextureManager()->Load("res/red_down.tga"));
		}

		crate = GetTextureManager()->Load("res/crate.tga");
		grass = GetTextureManager()->Load("res/grass.tga");

		crate->SetFilter(TF_BILINEAR);
		grass->SetFilter(TF_BILINEAR);

		cube = GetMeshManager()->LoadCube("cube", 1.0f);
		grid = GetMeshManager()->LoadGrid("grid", 32);

		font = GetFontManager()->Load("arial", "res/arialbd.ttf", 12);

		renderer->GetTextRenderer()->SetFont(font);
		renderer->GetTextRenderer()->SetColor(Color4::WHITE);

		ortho.BuildProjectionOrtho2dRH(1000, 625, -1, 1);
		crateModel.LoadIdentity();
		crateModel.SetTranslation(Vec3(0, 2, 0));
	}

	virtual void RenderScene()
	{
		Renderer* renderer = GetDevice()->GetRenderer();

		if (skybox)
		{
			skybox->SetVisible(optShowSkybox);
		}

		Scene::RenderScene();

		renderer->_SetLighting(false);
		renderer->_SetBlending(false);
		renderer->_SetAlphaTest(false);

		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		renderer->_SetTexturing(0, false);
		renderer->_SetTexturing(1, false);
		renderer->_SetTexturing(2, false);

		// Render crate
		if (optShowCrate)
		{
			renderer->_SetTexture(0, crate);
			renderer->_SetTexturing(0, true);
			renderer->_SetTextureMode(0, crate->GetMode(), crate->GetBlendColor());
			renderer->_SetTextureFilter(0, crate->GetFilter());

			renderer->SetMatrix(MT_MODEL, crateModel);
			renderer->RenderVertexIndexBuffer(cube->GetVertexBuffer(), cube->GetIndexBuffer());
		}

		// Render terrain
		if (optShowTerrain)
		{
			renderer->_SetTexture(0, grass);
			renderer->_SetTexturing(0, true);
			renderer->_SetTextureMode(0, grass->GetMode(), grass->GetBlendColor());
			renderer->_SetTextureFilter(0, grass->GetFilter());

			renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
			renderer->RenderVertexIndexBuffer(grid->GetVertexBuffer(), grid->GetIndexBuffer());
		}

/*
		renderer->SetMatrix(MT_MODEL,		Mat4::IDENTITY);
		renderer->SetMatrix(MT_VIEW,		Mat4::IDENTITY);
		renderer->SetMatrix(MT_PROJECTION,	mOrtho2dRH);

		renderer->_SetDepthTest(false);
		renderer->SetMatrix(MT_PROJECTION, ortho);
		renderer->SetMatrix(MT_MODEL, Mat4::IDENTITY);
		renderer->SetMatrix(MT_VIEW, Mat4::IDENTITY);
		renderer->GetTextRenderer()->Draw("Show/Hide Skybox (1)", 10, 24);
		renderer->GetTextRenderer()->Draw("Show/Hide Crate (2)", 10, 38);
		renderer->GetTextRenderer()->Draw("Show/Hide Terrain (3)", 10, 52);
		renderer->_SetDepthTest(true);
*/
	}

private:

	MovableCamera* cam;
	Skybox* skybox;
	Texture* crate;
	Texture* grass;
	Mesh* cube;
	Mesh* grid;
	Crown::Font* font;
	Mat4 ortho;
	Mat4 crateModel;

	bool optShowSkybox;
	bool optShowCrate;
	bool optShowTerrain;
};

int main(int argc, char** argv)
{
	Device* mDevice = GetDevice();

	if (!mDevice->Init(argc, argv))
	{
		return 0;
	}

	WndCtrl ctrl;

	MainScene* mainScene = new MainScene();
	GetDevice()->GetSceneManager()->SelectNextScene(mainScene);

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - Texturing Test");

	while (mDevice->IsRunning())
	{
		mDevice->Frame();
	}

	mDevice->Shutdown();

	return 0;
}

