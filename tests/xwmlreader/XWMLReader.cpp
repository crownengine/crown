#include "Crown.h"
#include <cstdlib>
#include <iostream>
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

class FruitInfo: public WithProperties
{
public:
	FruitInfo(Str name, float price):
		mName(name), mPrice(price)
	{
		AddProperty(new StrProperty("fruitName", &mName));
		AddProperty(new FloatProperty("fruitPrice", &mPrice));
	}

	virtual ~FruitInfo()
	{

	}

	Str ToStr() const
	{
		return mName;
	}

public:
	Str mName;
	float mPrice;
};

class AllWindowsContext: public WindowContext
{
public:
	AllWindowsContext(WindowsManager* windowsManager):
	  WindowContext(windowsManager)
  {
		mFruitsList = new List<Generic>();
		mFruitsList->Append(new FruitInfo("Pear", 1.38f));
		mFruitsList->Append(new FruitInfo("Apple", 2.41f));
		mFruitsList->Append(new FruitInfo("Peach", 1.89f));
		mFruitsList->Append(new FruitInfo("Tomato???", 0.87f));
		mFruitsList->Append(new FruitInfo("Orange", 1.40f));
		mFruitsList->Append(new FruitInfo("Pineapple", 2.15f));

		//AddProperty(new GenericListProperty("FruitsList", &mFruitsList));

		RegisterAction("IncreaseFruitPrice", CreateDelegate(this, &AllWindowsContext::IncreaseFruitPrice));
		RegisterAction("DecreaseFruitPrice", CreateDelegate(this, &AllWindowsContext::DecreaseFruitPrice));
  }

	virtual ~AllWindowsContext()
	{
		for(int i=0; i<mFruitsList->GetSize(); i++)
		{
			FruitInfo* fruitInfo = mFruitsList->GetElement(i).asType<FruitInfo>();
			delete fruitInfo;
		}
	}

	void OnLoad()
	{
		lwFruits = (ListView*)(GetAssociatedWindow()->FindChildByName("lwFruits"));
	}

	void IncreaseFruitPrice(Widget* src, List<Str>* /*args*/)
	{
		FruitPriceApplyDeltaToSelected(+1.0f);
	}

	void DecreaseFruitPrice(Widget* src, List<Str>* /*args*/)
	{
		FruitPriceApplyDeltaToSelected(-1.0f);
	}

	void FruitPriceApplyDeltaToSelected(float delta)
	{
		int selectedIndex = lwFruits->GetSelectedIndex();
		if (selectedIndex != -1)
		{
			FruitInfo* fruitInfo;
			mFruitsList->GetElement(selectedIndex).asType(&fruitInfo);
			fruitInfo->SetPropertyValue("fruitPrice", fruitInfo->mPrice + delta);
		}
	}

private:
	ListView* lwFruits;
	List<Generic>* mFruitsList;
};

class MainScene: public Scene, public WeakReferenced
{
public:
	MainScene(Crown::uint windowWidth, Crown::uint windowHeight)
	{

	}
	virtual ~MainScene()
	{

	}

	void LoadXWMLAndLogResponse(Str xwmlFile)
	{
		XWMLReader xwml;
		Window* window;

		Filesystem* fs = GetFilesystem();
		FilesystemEntry info;
//		if (!fs->GetInfo(xwmlFile, info))
//		{
//			MessageWindow* mw = new MessageWindow(mWindowsManager, "Load XWML", "Could not find file '" + xwmlFile + "'", MWR_OK, NULL);
//			mWindowsManager->DoModalWindow(mw);
//			return;
//		}

		window = xwml.LoadFile(xwmlFile, mWindowsManager, new AllWindowsContext(mWindowsManager));

		if (window == NULL)
			Log::E("Could not load XWML file '" + info.osPath + "'");
		else
			Log::I("Successfully loaded XWML file '" + info.osPath + "'");
	}

	virtual void OnLoad()
	{
		Scene::OnLoad();

		Renderer* renderer = GetDevice()->GetRenderer();
		renderer->SetClearColor(Color4(0.6f, 0.6f, 0.6f, 1.0f));

		mWindowsManager = new WindowsManager(this);
		mWindowsManager->RegisterAction("LoadXWMLFromFilepath", CreateDelegate(this, &MainScene::LoadXWMLFromFilepath));

		//LoadXWMLAndLogResponse("res/window.xml", wm);
		//LoadXWMLAndLogResponse("res/window_listview.xml", wm);
		LoadXWMLAndLogResponse("res/window_loader.xml");
	}

	void LoadXWMLFromFilepath(Widget* src, List<Str>* /*args*/)
	{
		Window* window = src->GetWindow();
		TextBox* tbXWMLFilepath = (TextBox*)window->FindChildByName("tbXWMLFilepath");
		if (tbXWMLFilepath == NULL)
		{
			Log::E("LoadXWMLFromFilepath action: Could not find TextBox 'tbXWMLFilepath'");
			return;
		}

		LoadXWMLAndLogResponse(tbXWMLFilepath->GetText());
	}

	virtual void RenderScene()
	{
		GetDevice()->GetRenderer()->_SetBackfaceCulling(false);

		mWindowsManager->Render();

		GetDevice()->GetRenderer()->_SetBackfaceCulling(true);
	}

private:
	WindowsManager* mWindowsManager;
}; 

int main(int argc, char** argv)
{
	int wndW = 1024;
	int wndH = 768;

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

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - XWMLReader Test");

	while (mDevice->IsRunning())
	{
		mDevice->Frame();
	}

	mDevice->Shutdown();

	return 0;
}

