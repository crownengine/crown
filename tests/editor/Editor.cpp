#include "Crown.h"
#include "CRWObjectModel.h"
#include <cstdlib>
#include <iostream>

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

void LoadXWMLAndLogResponse(WindowsManager* windowsManager, Str xwmlFile, WindowContext* context)
{
	XWMLReader xwml;
	Window* window;

	Filesystem* fs = GetFilesystem();
	FilesystemEntry info;
	if (!fs->GetInfo(xwmlFile, info))
	{
		MessageWindow* mw = new MessageWindow(windowsManager, "Load XWML", "Could not find file '" + xwmlFile + "'", MWR_OK, NULL);
		windowsManager->DoModalWindow(mw);
		return;
	}

	window = xwml.LoadFile(xwmlFile, windowsManager, context);

	if (window == NULL)
		Log::E("Could not load XWML file '" + info.osPath + "'");
	else
		Log::I("Successfully loaded XWML file '" + info.osPath + "'");
}

class CRWUtils
{
	public:
		static void CreateExampleCRW(Str crwFilePath)
		{
			CRWDecoder mDecoder;

			Stream* testimage = GetFilesystem()->OpenStream("res/terrain.bmp", SOM_READ);
			//FileStream* testfile = new FileStream("../../res/testfile.txt", SOM_READ);
			Stream* testfile = GetFilesystem()->OpenStream("res/testfile.txt", SOM_READ);

			mDecoder.New(crwFilePath);

			mDecoder.AddRawData("/", "file 1.txt", testfile, RF_NORMAL);
			testfile->Seek(0, SM_SeekFromBegin);

			mDecoder.AddDirectory("/", "docs");

			mDecoder.AddRawData("/docs", "file 2.txt", testfile, RF_NORMAL);
			testfile->Seek(0, SM_SeekFromBegin);

			mDecoder.AddDirectory("/docs", "images");
			mDecoder.AddRawData("/docs/images", "terrain.bmp", testimage, RF_NORMAL);
			mDecoder.AddDirectory("/docs", "todo");
			mDecoder.AddDirectory("/docs/todo", "asd1");
			mDecoder.AddDirectory("/docs/todo", "asd2");
			mDecoder.AddDirectory("/docs/images", "lll");
			mDecoder.AddDirectory("/docs/todo", "asd3");
			mDecoder.AddDirectory("/docs/todo", "asd4");
			mDecoder.AddDirectory("/docs/todo", "asd5");
			mDecoder.AddDirectory("/docs/todo", "asd6");

			for (int i=0; i<5; i++)
			{
				Str s = "info" + Str(i) + ".txt";
				mDecoder.AddRawData("/", s.c_str(), testfile, RF_NORMAL);
				testfile->Seek(0, SM_SeekFromBegin);
			}

			GetFilesystem()->Close(testimage);
			GetFilesystem()->Close(testfile);

			mDecoder.Close();
		}
};

//----------------------------------
//            Windows
//----------------------------------

class EditorWindow: public Window
{
  public:
    EditorWindow(WindowsManager* wm, int x, int y, int width, int height, const Str& crwFilePath):
      Window(wm, x, y, width, height, "Editor: " + crwFilePath)
    {
			mDecoder.Load(crwFilePath);
			mObjModel = new CRWObjectModel(&mDecoder);

			TreeView* tw = new TreeView(GetContentWidget());

			//The CRW Object Model exposes the CRW content through a hierarchy of IWithProperties objects
			//that is suitable to be viewed in a TreeView
			//Create a ListGenericWrapper to expose the list as a Generic List
			ListGenericWrapper<CRWDescriptor*>* wrapper = new ListGenericWrapper<CRWDescriptor*>(mObjModel->GetRoot());
			tw->SetItems(wrapper);
    }

		virtual ~EditorWindow()
		{
			delete mObjModel;
		}

  private:
		CRWDecoder mDecoder;
		CRWObjectModel* mObjModel;
};

class UpperCaseConverter: public Converter
{
public:
	virtual Generic Convert(Generic source)
	{
		Str str = source.asStr();
		str.MakeUpper();
		return str;
	}
};

class CrwIconConverter: public Converter
{
public:
	virtual Generic Convert(Generic source)
	{
		//source is a CRWDescriptor, return the correct icon ImageSource (a path, probably)
		ushort type;
		if (!source.asUShort(type))
		{
			return "<Unknown>";
		}
		if (type == RT_Directory)
		{
			return "res/crw_dir_icon.bmp";
		}
		if (type == RT_RawData)
		{
			return "res/crw_file_icon.bmp";
		}
		return "res/crw_unknown.bmp";
	}
};

class CrwDescriptorToFullPathConverter: public Converter
{
public:
	virtual Generic Convert(Generic source)
	{
		//source is a CRWDescriptor, return the correct icon ImageSource (a path, probably)
		CRWDescriptor* descr;
		if (!source.asType(&descr))
		{
			return "";
		}
		return descr->GetCRWObjectModel()->GetCRWLibraryPath() + descr->GetFullName();
	}
};

class EditorWindowContext: public WindowContext
{
public:
  EditorWindowContext(WindowsManager* windowsManager, const Str& crwFilePath):
		WindowContext(windowsManager), twCrwLibrary(NULL)
  {
		mDecoder.Load(crwFilePath);
		mObjModel = new CRWObjectModel(&mDecoder);

		AddProperty(new IWithPropertiesProperty("CrwObjectModel", (IWithProperties**)&mObjModel));

		RegisterAction("UpdateCrwFilePreview", CreateDelegate(this, &EditorWindowContext::UpdateCrwFilePreview));
		RegisterConverter("UpperCaseConverter", new UpperCaseConverter);
		RegisterConverter("CrwIconConverter", new CrwIconConverter);
		RegisterConverter("CrwDescriptorToFullPathConverter", new CrwDescriptorToFullPathConverter);
  }

	virtual ~EditorWindowContext()
	{
		delete mObjModel;
	}

	void OnLoad()
	{
		twCrwLibrary = (TreeView*)FindWidgetByName("twCrwLibrary");
		NotifyChangeEventArgs args("twCrwLibrary");
		NotifyChange(&args);
	}

	virtual Generic GetPropertyValue(const Str& name) const
	{
		if (name == "twCrwLibrary")
		{
			return twCrwLibrary;
		}
		else
		{
			return WindowContext::GetPropertyValue(name);
		}
	}

	
	void UpdateCrwFilePreview(Widget* src, List<Str>* /*args*/)
	{
		//if (twCrwLibrary->GetSelectedIndex() != 2)
		//{
		//	twCrwLibrary->SetSelectedIndex(2);
		//}
		//twCrwLibrary->PrintLoop(0);
	}

	virtual Str ToStr() const
	{
		return "EditorWindowContext";
	}

private:
	TreeView* twCrwLibrary;
	CRWDecoder mDecoder;
	CRWObjectModel* mObjModel;
};

class MainWindowContext: public WindowContext
{
  public:
    MainWindowContext(WindowsManager* windowsManager):
		  WindowContext(windowsManager)
    {
			RegisterAction("NewLibrary", CreateDelegate(this, &MainWindowContext::NewLibraryAction));
			RegisterAction("OpenLibrary", CreateDelegate(this, &MainWindowContext::OpenLibraryAction));
    }

		void OnLoad()
		{
			btnClose = (Button*)FindWidgetByName("btnClose");
			btnNewLibrary = (Button*)FindWidgetByName("btnNewLibrary");
			btnOpenLibrary = (Button*)FindWidgetByName("btnOpenLibrary");
		}

		void NewLibraryAction(Widget* src, List<Str>* /*args*/)
		{
			Window* wnd = new TextInputWindow(
				GetWindowsManager(),
				"New CRW", "Please insert the new CRW name:",
				"library",
				new Delegate2<MainWindowContext, void, bool, Str>(this, &MainWindowContext::NewLibraryCallback, false, Str(""))
				);
			GetWindowsManager()->DoModalWindow(wnd);
		}

		void OpenLibraryAction(Widget* src, List<Str>* /*args*/)
		{
			Window* wnd = new TextInputWindow(
				GetWindowsManager(),
				"Open CRW", "Please insert the CRW name to open:",
				"library",
				new Delegate2<MainWindowContext, void, bool, Str>(this, &MainWindowContext::OpenCrwCallback, false, Str(""))
				);
			GetWindowsManager()->DoModalWindow(wnd);
		}

		void NewLibraryCallback(bool cancelled, Str name)
		{
			if (!cancelled)
			{
				Log::I("Creating new CRW: " + name + ".crw");

				Str crwFilePath = "res/" + name + ".crw";

				CRWUtils::CreateExampleCRW(crwFilePath);
				new EditorWindow(GetWindowsManager(), 100, 15, 250, 200, crwFilePath);
			}
			else
				Log::I("New CRW creation cancelled.");
		}

		void OpenCrwCallback(bool cancelled, Str name)
		{
			if (!cancelled)
			{
				Log::I("Opening CRW: " + name + ".crw");

				Str crwFilePath = "res/" + name + ".crw";

				//Verify the existence of crwFilePath
				if (!GetFilesystem()->Exists(crwFilePath))
				{
					Log::I("CRW file specified does not exist");
				}
				else
				{
					//new EditorWindow(GetWindowsManager(), 100, 15, 250, 200, crwFilePath);
					WindowsManager* windowsManager = GetWindowsManager();
					LoadXWMLAndLogResponse(windowsManager, "res/editor_editorwindow.xml", new EditorWindowContext(windowsManager, crwFilePath));
				}
			}
			else
				Log::I("Opening CRW cancelled.");
		}

	private:
		Button* btnClose;
		Button* btnNewLibrary;
		Button* btnOpenLibrary;
};

class MainScene: public Scene
{
public:
	MainScene(Crown::uint windowWidth, Crown::uint windowHeight)
	{
	}

	virtual ~MainScene()
	{
	}

	virtual void OnLoad()
	{
		Renderer* renderer = GetDevice()->GetRenderer();
		renderer->SetClearColor(Color4(0.6f, 0.6f, 0.6f, 1.0f));

		//Mat4 matrix(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f);
		//renderer->SetMatrix(MT_MODEL, matrix);

		//Mat4 matrix1;
		//glGetFloatv(GL_MODELVIEW_MATRIX, matrix1.ToFloatPtr());

		mWindowsManager = new WindowsManager(this);

		LoadXWMLAndLogResponse(mWindowsManager, "res/editor_mainwindow.xml", new MainWindowContext(mWindowsManager));
	}

	virtual void RenderScene()
	{
		Scene::RenderScene();
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

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - Editor Test");

	mDevice->Run();

	mDevice->Shutdown();

	return 0;
}



