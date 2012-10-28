#include "Crown.h"
#include <cstdlib>
#include <iostream>
#include <GL/glew.h>

//----------------------------------
//            Windows
//----------------------------------

class MainWindow: public Crown::Window
{
  public:
    MainWindow(Crown::WindowsManager* wm, float x, float y, float width, float height):
      Crown::Window(wm, x, y, width, height, "Crown")
    {
			Crown::StackLayout* sl = new Crown::StackLayout(GetContentWidget());

			btnClose = new Crown::Button(sl);
			btnClose->SetDesiredSize(-1, -1);

			/*Crown::ThemeSpriteWidget* tsw =*/ new Crown::ThemeSpriteWidget(btnClose, Crown::TS_BOOM, 0);

			btnClose->OnClickEvent += CreateDelegate(this, &MainWindow::btnClose_OnClick);
    }

		void btnClose_OnClick(Button* /*obj*/, Crown::EventArgs* /*args*/)
		{
			Crown::GetDevice()->StopRunning();
		}

  private:
		Crown::Button* btnClose;
};

class MyWindow: public Crown::Window
{
  public:
    MyWindow(Crown::WindowsManager* wm, float x, float y, float width, float height):
      Crown::Window(wm, x, y, width, height, "Test Window")
    {
			Crown::StackLayout* sl = new Crown::StackLayout(GetContentWidget());
			Crown::StackLayout* sl_left = new Crown::StackLayout(sl);
			Crown::StackLayout* sl_right = new Crown::StackLayout(sl);
			sl->SetOrientationVertical(false);

      /*Crown::Button* btn1 =*/ new Crown::Button(sl_left);
			Crown::Button* btn2 = new Crown::Button(sl_left);
			/*Crown::Button* btn3 =*/ new Crown::Button(sl_left);
			Crown::Button* btn4 = new Crown::Button(sl_left);

			Crown::Button* btn5 = new Crown::Button(sl_right);
			/*Crown::Button* btn6 =*/ new Crown::Button(sl_right);
			Crown::Button* btn7 = new Crown::Button(sl_right);
			/*Crown::Button* btn8 =*/ new Crown::Button(sl_right);

			btn2->SetDesiredSize(-1, -1);
			btn4->SetDesiredSize(-1, -1);
			btn5->SetDesiredSize(-1, -1);
			btn7->SetDesiredSize(-1, -1);
    }

  private:
};

class ListWindow: public Crown::Window
{
  public:
    ListWindow(Crown::WindowsManager* wm, float x, float y, float width, float height):
      Crown::Window(wm, x, y, width, height, "ListView Window")
    {
			Crown::StackLayout* sl = new Crown::StackLayout(GetContentWidget());
			sl->SetOrientationVertical(false);

			Crown::ListView* lw = new Crown::ListView(sl);

//			Crown::GenericList gl;
//      
//      for(int i = 0; i < 15; i++)
//        gl.Append(i*3);

//			lw->SetItems(gl);


			Crown::StackLayout* slRight = new Crown::StackLayout(sl);

			Crown::TreeView* treeview = new Crown::TreeView(slRight);
    }

  private:
};

class MainScene: public Crown::Scene
{
	public:
		MainScene() {}
		virtual ~MainScene()
		{
			if (tr)
				delete tr;
		}

		virtual void OnLoad()
		{
			Crown::Renderer* renderer = Crown::GetDevice()->GetRenderer();
			renderer->SetClearColor(Crown::Color(0.6f, 0.6f, 0.6f, 1.0f));

			Crown::WindowsManager* wm = new Crown::WindowsManager(this);
			/*Crown::Window* w1 =*/ new MainWindow(wm, 10.0f, 15.0f, 80.0f, 80.0f);
			/*Crown::Window* w2 =*/ new MyWindow(wm, 150.0f, 150.0f, 100.0f, 100.0f);
			/*Crown::Window* w3 =*/ new ListWindow(wm, 350.0f, 150.0f, 300.0f, 200.0f);

			/*
			mFont.SetTrueTypeSize(8);
			mImg = mFont.LoadFont("../../res/times.ttf");
			
			Crown::Frame* f = new Crown::Frame();
			f->Set(renderer->CreateTexture(mImg), 0, 0, mImg->GetWidth(), mImg->GetHeight());
			mSprite.AddFrame(f);

			Crown::BMPImageLoader bmp;
			bmp.SaveFile(mImg, "../../res/out.bmp");
			*/
		}

		virtual void RenderScene()
		{
			glDisable(GL_LIGHTING);
			glColor3f(1, 1, 1);

			// Uncomment to gain a huge performance boost
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);

			Scene::RenderScene();

			//glTranslatef(0.0f, 100.0f, 0.0f);
			//mSprite.draw(0);
		}

		//Crown::Image* mImg;
		//Crown::Sprite mSprite;
};

int main(int argc, char** argv) {

	int wndW, wndH;
	wndW = 800;
	wndH = 600;

	if (argc == 3) {
		wndW = atoi(argv[1]);
		wndH = atoi(argv[2]);
	}

	Crown::Device* device = Crown::GetDevice();

	if (!device->Init(wndW, wndH, 32, false)) {

		return 0;
	}

	device->GetMainWindow()->SetTitle("Crown Engine v0.1");

	MainScene* mainScene = new MainScene();
	device->GetSceneManager()->SelectNextScene(mainScene);
	
	device->Run();
	device->Shutdown();

	return 0;

}
