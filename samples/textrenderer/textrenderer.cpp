#include "Crown.h"
#include <cstdlib>
#include <GL/glew.h>

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

class MainScene
{

public:

	MainScene(uint windowWidth, uint windowHeight)
	{
	}

	virtual ~MainScene()
	{
	}

	virtual void OnLoad()
	{
		mOrtho.BuildProjectionOrtho2dRH(800, 600, 10, -10);

		Crown::Renderer* renderer = Crown::GetDevice()->GetRenderer();

		renderer->SetClearColor(Crown::Color4(0.8f, 0.8f, 0.8f, 0.8f));

		// Load and select the font
		mLargeFont = GetFontManager()->Load("res/arial.tga");

		renderer->_SetLighting(false);
	}

	virtual void RenderScene()
	{
		Scene::RenderScene();

		Renderer* renderer = GetDevice()->GetRenderer();

		Scene::RenderScene();

		renderer->SetMatrix(MT_PROJECTION, mOrtho);

		//text.SetFont(mLargeFont);
		//text.SetColor(Color4::SILVER);
		text.Draw("abcdef ABCDEF .@#^?=()&%!", 100, 90, mLargeFont);
		text.Draw("Hello world! This is the new TextRenderer.\n", 100, 120, mLargeFont);
		text.Draw("It supports a lot of new features!", 100, 150, mLargeFont);
	}

private:

	Mat4 mOrtho;
	Crown::Font* mLargeFont;
	Crown::Font* mSmallFont;
	Crown::Font* mTimesFont;

	TextRenderer text;
};

int main(int argc, char** argv)
{
	int wndW = 800;
	int wndH = 600;

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

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - TextRenderer Test");

	while (mDevice->IsRunning())
	{
		mDevice->Frame();
	}

	mDevice->Shutdown();

	return 0;
}

