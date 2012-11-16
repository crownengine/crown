/*
Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include "LunaTheme.h"
#include "Themes.h"
#include <GL/glew.h> // FIXME
#include "Types.h"
#include "Color4.h"
#include "BMPImageLoader.h"
#include "Device.h"
#include "Image.h"
#include "TextureManager.h"
#include "Renderer.h"

namespace Crown
{

LunaTheme::LunaTheme()
{
	mFunctions[TF_BUTTON_NORMAL] = &LunaTheme::DrawButtonNormal;
	mFunctions[TF_BUTTON_HIGHLIGHTED] = &LunaTheme::DrawButtonHighlighted;
	mFunctions[TF_BUTTON_PRESSED] = &LunaTheme::DrawButtonPressed;
	mFunctions[TF_WINDOW] = &LunaTheme::DrawWindow;
	mFunctions[TF_LISTVIEW] = &LunaTheme::DrawListView;

	BMPImageLoader loader;
	Image* img;
	Frame* f;
	Sprite* sprite;
	img = loader.LoadFile("res/closebutton_x.bmp");
	img->ApplyColorKeying(Color4(255, 0, 255));
	f = new Frame();

	bool created;
	Texture* tex1;
	tex1 = GetTextureManager()->Create("closebutton", created);
	tex1->LoadFromImage(img);
	f->Set(tex1, 0, img->GetHeight()-3, 3, img->GetHeight());

	sprite = new Sprite();
	sprite->AddFrame(f);
	mSprites[TS_CLOSEBUTTON_X] = sprite;
	delete img;

	img = loader.LoadFile("res/boom.bmp");
	img->ApplyColorKeying(Color4(255, 255, 255));
	f = new Frame();

	Texture* tex2;
	tex2 = GetTextureManager()->Create("boom", created);
	tex2->LoadFromImage(img);
	f->Set(tex2, 0, 0, img->GetWidth(), img->GetHeight());

	sprite = new Sprite();
	sprite->AddFrame(f);
	mSprites[TS_BOOM] = sprite;
	delete img;
}

LunaTheme::~LunaTheme()
{
	SpritesDictionary::Enumerator e = mSprites.getBegin();

	while (e.next())
	{
		delete e.current().value;
	}
}

ThemeFunction LunaTheme::GetFunction(ThemesFunctions functionId)
{
	return mFunctions[functionId];
}

Sprite* LunaTheme::GetSprite(ThemesSprites spriteId)
{
	return mSprites[spriteId];
}

void LunaTheme::DrawButtonNormal(const Point32_t2& size)
{
	glPushMatrix();
	int32_t fWidth = size.x;
	int32_t fHeight = size.y;
	glBegin(GL_QUAD_STRIP);
	glColor3ub(245, 245, 245);
	glVertex2i(fWidth-1, 1);
	glVertex2i(1       , 1);
	glColor3ub(220, 220, 220);
	glVertex2i(fWidth-1, int32_t((fHeight-1)*0.55));
	glVertex2i(1       , int32_t((fHeight-1)*0.55));
	glColor3ub(200, 200, 200);
	glVertex2i(fWidth-1, int32_t((fHeight-1)*0.8));
	glVertex2i(1       , int32_t((fHeight-1)*0.8));
	glColor3ub(210, 210, 210);
	glVertex2i(fWidth-1, fHeight-1);
	glVertex2i(1       , fHeight-1);
	glEnd();
	glBegin(GL_LINES);
	glColor3f(0.85f, 0.85f, 0.85f);
	glVertex2i(1       , 0);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(fWidth-1, 0);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(1       , fHeight-1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex2i(fWidth-1, fHeight-1);
	glColor3f(0.85f, 0.85f, 0.85f);
	glVertex2i(0       , 1);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(0       , fHeight-1);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(fWidth-1, 1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex2i(fWidth-1, fHeight-1);
	glEnd();
	glPopMatrix();
}

void LunaTheme::DrawButtonHighlighted(const Point32_t2& size)
{
	glPushMatrix();
	int32_t fWidth = size.x;
	int32_t fHeight = size.y;
	glBegin(GL_QUAD_STRIP);
	glColor3ub(250, 250, 250);
	glVertex2i(fWidth-1, 1);
	glVertex2i(1       , 1);
	glColor3ub(225, 225, 225);
	glVertex2i(fWidth-1, int32_t((fHeight-1)*0.55));
	glVertex2i(1       , int32_t((fHeight-1)*0.55));
	glColor3ub(205, 205, 205);
	glVertex2i(fWidth-1, int32_t((fHeight-1)*0.8));
	glVertex2i(1       , int32_t((fHeight-1)*0.8));
	glColor3ub(215, 215, 215);
	glVertex2i(fWidth-1, fHeight-1);
	glVertex2i(1       , fHeight-1);
	glEnd();
	glBegin(GL_LINES);
	glColor3f(0.90f, 0.90f, 0.90f);
	glVertex2i(1       , 0);
	glColor3f(0.80f, 0.80f, 0.80f);
	glVertex2i(fWidth-1, 0);
	glColor3f(0.80f, 0.80f, 0.80f);
	glVertex2i(1       , fHeight-1);
	glColor3f(0.55f, 0.55f, 0.55f);
	glVertex2i(fWidth-1, fHeight-1);
	glColor3f(0.90f, 0.90f, 0.90f);
	glVertex2i(0       , 1);
	glColor3f(0.80f, 0.80f, 0.80f);
	glVertex2i(0       , fHeight-1);
	glColor3f(0.80f, 0.80f, 0.80f);
	glVertex2i(fWidth-1, 1);
	glColor3f(0.55f, 0.55f, 0.55f);
	glVertex2i(fWidth-1, fHeight-1);
	glEnd();
	glPopMatrix();
}

void LunaTheme::DrawButtonPressed(const Point32_t2& size)
{
	glPushMatrix();
	int32_t fWidth = size.x;
	int32_t fHeight = size.y;
	glBegin(GL_QUAD_STRIP);
	glColor3ub(210, 210, 210);
	glVertex2i(fWidth-1, 1);
	glVertex2i(1       , 1);
	glColor3ub(200, 200, 200);
	glVertex2i(fWidth-1, int32_t((fHeight-1)*0.2));
	glVertex2i(1       , int32_t((fHeight-1)*0.2));
	glColor3ub(220, 220, 220);
	glVertex2i(fWidth-1, int32_t((fHeight-1)*0.45));
	glVertex2i(1       , int32_t((fHeight-1)*0.45));
	glColor3ub(245, 245, 245);
	glVertex2i(fWidth-1, fHeight-1);
	glVertex2i(1       , fHeight-1);
	glEnd();
	glBegin(GL_LINES);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex2i(1       , 0);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(fWidth-1, 0);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(1       , fHeight-1);
	glColor3f(0.85f, 0.85f, 0.85f);
	glVertex2i(fWidth-1, fHeight-1);
	glColor3f(0.5f, 0.5f, 0.5f);
	glVertex2i(0       , 1);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(0       , fHeight-1);
	glColor3f(0.75f, 0.75f, 0.75f);
	glVertex2i(fWidth-1, 1);
	glColor3f(0.85f, 0.85f, 0.85f);
	glVertex2i(fWidth-1, fHeight-1);
	glEnd();
	glPopMatrix();
}

void LunaTheme::DrawWindow(const Point32_t2& size)
{
	glPushMatrix();
	int32_t fWidth = size.x;
	int32_t fHeight = size.y;
	glColor3ub(235, 235, 235);
	glBegin(GL_QUAD_STRIP);
	glVertex2i(fWidth-1, 1);
	glVertex2i(1       , 1);
	glColor3ub(220, 220, 220);
	glVertex2i(fWidth-1, fHeight-1);
	glVertex2i(1       , fHeight-1);
	glEnd();
	const float alphaVal = 1.0f;
	glBegin(GL_LINES);
	glColor4f(0.8f, 0.8f, 0.8f, alphaVal);
	//glColor4f(1.0f, 1.0f, 1.0f, alphaVal);
	glVertex2i(1       , 0);
	//glColor4f(0.8f, 0.8f, 0.8f, alphaVal);
	glVertex2i(fWidth-1, 0);
	//glColor4f(0.8f, 0.8f, 0.8f, alphaVal);
	glVertex2i(1       , fHeight-1);
	//glColor4f(0.6f, 0.6f, 0.6f, alphaVal);
	glVertex2i(fWidth-1, fHeight-1);
	//glColor4f(1.0f, 1.0f, 1.0f, alphaVal);
	glVertex2i(0       , 1);
	//glColor4f(0.8f, 0.8f, 0.8f, alphaVal);
	glVertex2i(0       , fHeight-1);
	//glColor4f(0.8f, 0.8f, 0.8f, alphaVal);
	glVertex2i(fWidth-1, 1);
	//glColor4f(0.6f, 0.6f, 0.6f, alphaVal);
	glVertex2i(fWidth-1, fHeight-1);
	glEnd();
	glPopMatrix();
}

void LunaTheme::DrawListView(const Point32_t2& size)
{
	glPushMatrix();
	int32_t fWidth = size.x;
	int32_t fHeight = size.y;
	glColor3ub(245, 245, 255);
	glBegin(GL_QUAD_STRIP);
	glVertex2i(fWidth-1, 1);
	glVertex2i(1       , 1);
	glColor3ub(230, 230, 240);
	glVertex2i(fWidth-1, fHeight-1);
	glVertex2i(1       , fHeight-1);
	glEnd();
	glPopMatrix();

//	const float alphaVal = 1.0f;
//	glBegin(GL_LINES);
//	glColor4f(0.75f, 0.75f, 1.0f, alphaVal);
//	glVertex2i(1       , 0);
//	glVertex2i(fWidth-1, 0);
//	glVertex2i(1       , fHeight-1);
//	glVertex2i(fWidth-1, fHeight-1);
//	glVertex2i(0       , 1);
//	glVertex2i(0       , fHeight-1);
//	glVertex2i(fWidth-1, 1);
//	glVertex2i(fWidth-1, fHeight-1);
//	glEnd();
//	glPopMatrix();
}

} //namespace Crown
