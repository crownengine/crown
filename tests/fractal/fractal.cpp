#include <cstdlib>
#include <iostream>
#include "TestApp.h"
#include "TestScene.h"

using namespace Crown;

Image* gImage = NULL;
TexturePtr gTexture;
Sprite* gSprite;
float aspectRatio;
float minX;
float maxX;
float minY;
float maxY;
float sizeX;
float sizeY;
float panX;
float panY;

float centerX;
float centerY;

class AllWindowsContext: public WindowContext, public WeakReferenced
{
public:
	AllWindowsContext(WindowsManager* windowsManager):
	  WindowContext(windowsManager)
  {
		sizeX = 2.0f;
		centerX = 0.0f;
		centerY = 0.0f;
		CalculateSizeParams();

		RegisterAction("Generate", CreateDelegate(this, &AllWindowsContext::Generate));
		RegisterAction("MoveLeft", CreateDelegate(this, &AllWindowsContext::MoveLeft));
		RegisterAction("MoveRight", CreateDelegate(this, &AllWindowsContext::MoveRight));
		RegisterAction("MoveUp", CreateDelegate(this, &AllWindowsContext::MoveUp));
		RegisterAction("MoveDown", CreateDelegate(this, &AllWindowsContext::MoveDown));

		RegisterAction("ZoomPlus", CreateDelegate(this, &AllWindowsContext::ZoomPlus));
		RegisterAction("ZoomMinus", CreateDelegate(this, &AllWindowsContext::ZoomMinus));
  }

	virtual ~AllWindowsContext()
	{

	}

	void OnLoad()
	{
		GetAssociatedWindow()->SetBindingContext(this);
	}

	void MoveLeft(Widget* /*src*/, List<Str>* /*args*/)
	{
		centerX -= panX;
		CalculateSizeParams();
		Regenerate();
	}

	void MoveRight(Widget* /*src*/, List<Str>* /*args*/)
	{
		centerX += panX;
		CalculateSizeParams();
		Regenerate();
	}

	void MoveUp(Widget* /*src*/, List<Str>* /*args*/)
	{
		centerY += panY;
		CalculateSizeParams();
		Regenerate();
	}

	void MoveDown(Widget* /*src*/, List<Str>* /*args*/)
	{
		centerY -= panY;
		CalculateSizeParams();
		Regenerate();
	}

	void ZoomPlus(Widget* /*src*/, List<Str>* /*args*/)
	{
		sizeX /= 1.3f;
		CalculateSizeParams();
		Regenerate();
	}

	void ZoomMinus(Widget* /*src*/, List<Str>* /*args*/)
	{
		sizeX *= 1.3f;
		CalculateSizeParams();
		Regenerate();
	}

	void Regenerate()
	{
		Generate(NULL, NULL);
	}

	void Generate(Widget* /*src*/, List<Str>* /*args*/)
	{
		/*Color c;
		c.r = (rand() % 1000) / 1000.0f;
		c.g = (rand() % 1000) / 1000.0f;
		c.b = (rand() % 1000) / 1000.0f;
		gImage->SetUniformColorImage(c);*/

		float cX = 0.0f;
		float cY = 0.0f;

		int maxIterations = 150;

		for(uint py = 0; py < gImage->GetHeight(); py++)
		{
			cY = minY + py * sizeY / gImage->GetHeight();
			for(uint px = 0; px < gImage->GetWidth(); px++)
			{
				cX = minX + px * sizeX / gImage->GetWidth();

				//Iterate some times
				float x = 0.0f;
				float y = 0.0f;
				int i = 0;
				while (i < maxIterations && (x * x + y * y < 2))
				{
					float xTmp = x * x - y * y + cX;
					y = 2*x*y + cY;
					x = xTmp;

					i++;
				}

				if (i < maxIterations)
				{
					int comp = (int)(i * 255.0f / maxIterations);
					gImage->SetPixel(px, py, Color(comp, comp, 255 - comp));
				}
				else
				{
					gImage->SetPixel(px, py, Color::BLACK);
				}
			}
		}

		gTexture->LoadFromImage(gImage);
	}

	void CalculateSizeParams()
	{
		sizeY = sizeX * aspectRatio;
		minX = -sizeX / 2.0f + centerX;
		maxX = +sizeX / 2.0f + centerY;

		minY = -sizeY / 2.0f + centerY;
		maxY = +sizeY / 2.0f + centerY;

		panX = sizeX / 10.0f;
		panY = sizeY / 10.0f;
	}

private:
	
};

class MainScene: public TestScene, public WeakReferenced
{
public:
	MainScene(Crown::uint windowWidth, Crown::uint windowHeight) :
		TestScene(windowWidth, windowHeight)
	{
		aspectRatio = (float)windowHeight / windowWidth;
		sizeX = 2.0f;
		sizeY = sizeX * aspectRatio;
		minX = -sizeX / 2.0f;
		maxX = +sizeX / 2.0f;

		minY = -sizeY / 2.0f;
		maxY = +sizeY / 2.0f;

		panX = sizeX / 10.0f;
		panY = sizeY / 10.0f;

		gImage = new Image(IT_2D, PF_RGB_8, windowWidth, windowHeight, NULL);
		bool created;
		gTexture = GetDevice()->GetRenderer()->GetTextureManager()->Create("fractal_tex", created);
		gTexture->LoadFromImage(gImage);

		gSprite = new Sprite();
		Frame* f = new Frame();
		f->Set(gTexture);
		gSprite->AddFrame(f);
	}
	virtual ~MainScene()
	{

	}

	void LoadXWMLAndLogResponse(Str xwmlFile)
	{
		XWMLReader xwml;
		Window* window;

		Filesystem* fs = GetDevice()->GetFilesystem();
		FilesystemEntry info;
		if (!fs->GetInfo(xwmlFile, info))
		{
			MessageWindow* mw = new MessageWindow(mWindowsManager, "Load XWML", "Could not find file '" + xwmlFile + "'", MWR_OK, NULL);
			mWindowsManager->DoModalWindow(mw);
			return;
		}

		window = xwml.LoadFile(xwmlFile, mWindowsManager, new AllWindowsContext(mWindowsManager));

		if (window == NULL)
			Log::E("Could not load XWML file '" + info.GetAbsolutePath() + "'");
		else
			Log::I("Successfully loaded XWML file '" + info.GetAbsolutePath() + "'");
	}

	virtual void OnLoad()
	{
		TestScene::OnLoad();

		Renderer* renderer = GetDevice()->GetRenderer();
		renderer->SetClearColor(Color(0.6f, 0.6f, 0.6f, 1.0f));

		mWindowsManager = new WindowsManager(this);

		LoadXWMLAndLogResponse("res/window_fractal.xml");
	}

	virtual void RenderScene()
	{
		gSprite->draw(0);

		TestScene::RenderScene();
	}

private:
	WindowsManager* mWindowsManager;
}; 

CROWN_APP_AND_MAIN(TestApp, MainScene, 1024, 768, "Crown Engine v0.1 - XWMLReader Test");
