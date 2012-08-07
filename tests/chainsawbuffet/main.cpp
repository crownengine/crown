#include "ArenaScene.h"

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

int main(int argc, char** argv)
{
	int wndW = 800;
	int wndH = 480;

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

	ArenaScene* mainScene = new ArenaScene(wndW, wndH);
	GetDevice()->GetSceneManager()->SelectNextScene(mainScene);

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - Chainsaw Buffet");

	while (mDevice->IsRunning())
	{
		mDevice->Frame();
	}

	mDevice->Shutdown();

	return 0;
}

