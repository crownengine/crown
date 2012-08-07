#include "Crown.h"
#include <cstdlib>
#include <GL/glew.h>

using namespace Crown;

void DrawCircle(const Circle& circle);
void DrawLine(const Vec2& start, const Vec2& end);
void DrawRectangle(const Crown::Rect& rect, const Color4& color);

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

class MainScene: public Scene, public MouseListener//, public KeyboardListener
{

public:

	MainScene(uint windowWidth, uint windowHeight) :
		c1(Vec2::ZERO, 0.0f),
		c2(Vec2::ZERO, 0.0f)
	{
		GetDevice()->GetInputManager()->RegisterMouseListener(this);
		//GetDevice()->GetInputManager()->RegisterKeyboardListener(this);
	}

	virtual ~MainScene()
	{
	}

	virtual void ButtonPressed(const MouseEvent& event)
	{
		
	}

	virtual void OnLoad()
	{
		Crown::Renderer* renderer = Crown::GetDevice()->GetRenderer();
		renderer->SetClearColor(Crown::Color4(0.6f, 0.6f, 0.6f, 1.0f));

		c1.SetCenter(Vec2(300.0f, 300.0f));
		c1.SetRadius(32.0f);

		c2.SetCenter(Vec2(500.0f, 700.0f));
		c2.SetRadius(16.0f);

		ortho.BuildProjectionOrtho2dRH(1000, 625, 0, 10);
	}

	virtual void RenderScene()
	{
		Renderer* renderer = GetDevice()->GetRenderer();

		renderer->_SetLighting(false);
		renderer->_SetBlending(false);
		renderer->_SetAlphaTest(false);
		renderer->_SetBackfaceCulling(false);
		renderer->_SetTexturing(0, false);
		renderer->_SetTexturing(1, false);
		renderer->_SetTexturing(2, false);

		Scene::RenderScene();

		renderer->SetMatrix(MT_PROJECTION, ortho);

		glDisable(GL_CULL_FACE);

		glColor4f(0, 1, 0, 1);

		Crown::Rect r1;
		r1.SetFromCenterAndDimensions(Vec2(400, 400), 100, 80);

		c1.SetCenter(GetDevice()->GetInputManager()->GetMouse()->GetCursorXY().ToVec2());

		Color4 color = Color4::LIME;

		Vec2 penetration;
		if (Intersection::TestCircleRect(c1, r1, penetration))
		{
			color = Color4::RED;
		}

		Circle cr1 = r1.ToCircle();
		DrawCircle(cr1);

		Rect rc1 = c1.ToRect();
		DrawRectangle(rc1, color);

		DrawRectangle(r1, color);
		DrawCircle(c1);

		glColor4f(0, 0, 1, 1);
		DrawLine(c1.GetCenter(), c1.GetCenter() + penetration);

		//DrawCircle(c2);

		//Vec2 c1vel = GetDevice()->GetInputManager()->GetMouse()->GetCursorXY().ToVec2() - c1.GetCenter();
		//Vec2 c2vel = Vec2(250.0f, -200.0f);

		//DrawLine(c1.GetCenter(), c1.GetCenter() + c1vel);
		//DrawLine(c2.GetCenter(), c2.GetCenter() + c2vel);

		//real ct;
		//if (Intersection::TestDynamicCircleCircle(c1, c1vel, c2, c2vel, ct))
		//{
		//	glColor4f(1, 0, 0, 1);

		//	Circle cc1(c1.GetCenter() + c1vel * ct, c1.GetRadius());
		//	Circle cc2(c2.GetCenter() + c2vel * ct, c2.GetRadius());

		//	DrawCircle(cc1);
		//	DrawCircle(cc2);
		//}
	}

private:

	Circle c1;
	Circle c2;
	Mat4 ortho;
};

int main(int argc, char** argv)
{
	int wndW = 1000;
	int wndH = 625;

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

	mDevice->GetMainWindow()->SetTitle("Crown Engine v0.1 - Collision 2D Test");

	mDevice->Run();

	mDevice->Shutdown();

	return 0;
}

void DrawCircle(const Circle& circle)
{
	float step = (360.0f / 16.0f) * Math::DEG_TO_RAD;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2fv(circle.GetCenter().ToFloatPtr());
	for (int i = 0; i <= 16; i++)
	{
		Vec2 vert = circle.GetCenter() + (Vec2(Math::Cos((float)i * step), Math::Sin((float)i * step)) * circle.GetRadius());
		glVertex2fv(vert.ToFloatPtr());
	}
	glEnd();
}

void DrawLine(const Vec2& start, const Vec2& end)
{
	glBegin(GL_LINES);
	glVertex2fv(start.ToFloatPtr());
	glVertex2fv(end.ToFloatPtr());
	glEnd();
}

void DrawRectangle(const Crown::Rect& rect, const Color4& color)
{
	Renderer* r = GetDevice()->GetRenderer();
	
	Vec2 min = rect.GetVertext(0);
	Vec2 size = rect.GetSize();
	r->DrawRectangle(Point2(min.x, min.y), Point2(size.x, size.y), DM_BORDER, color);
}

