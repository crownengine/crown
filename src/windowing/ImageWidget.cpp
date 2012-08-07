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

#include "ImageWidget.h"
#include "TextureManager.h"
#include "SpecificProperties.h"
#include "BMPImageLoader.h"
#include "Renderer.h"

namespace Crown
{

ImageWidget::ImageWidget(Widget* parent):
	Widget(parent)
{
	AddProperty(new StrProperty("ImageSource", &mImageSource));
}

ImageWidget::ImageWidget(Widget* parent, const Str& imageSource):
	Widget(parent), mImageSource(imageSource)
{
	AddProperty(new StrProperty("ImageSource", &mImageSource));
	ReloadFromImageSource();
}

ImageWidget::~ImageWidget()
{
}

void ImageWidget::OnSetProperty(const Str& name)
{
	if (name == "ImageSource")
	{
		ReloadFromImageSource();
	}
}

void ImageWidget::ReloadFromImageSource()
{
	mSprite.Clear();

	Texture* tex;
	bool created;
	tex = GetTextureManager()->Create(mImageSource.c_str(), created);
	if (created)
	{
		BMPImageLoader loader;
		Image* img;

		img = loader.LoadFile(mImageSource);
		if (img == NULL)
		{
			return;
		}
		img->ApplyColorKeying(Color4(255, 0, 255));

		tex->LoadFromImage(img);
		delete img;
	}

	Frame* frame = new Frame();
	frame->Set(tex);
	mSprite.AddFrame(frame);
}

void ImageWidget::OnDraw(DrawingClipInfo& clipInfo)
{
	if (mSprite.GetFrameCount() > 0)
	{
		mSprite.draw(0);
	}
	Widget::OnDraw(clipInfo);
}

} //namespace Crown
