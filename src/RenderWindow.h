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

#pragma once

#include "Types.h"
#include "Str.h"

namespace Crown
{

/**
	Rendering window interface.

	The primary target for render operations.
*/
class RenderWindow
{

public:

	static RenderWindow*	CreateWindow(uint x, uint y, uint width, uint height, uint depth, bool fullscreen);
	static void				DestroyWindow(RenderWindow* window);

							RenderWindow();									//!< Constructor
	virtual					~RenderWindow();								//!< Destructor

	virtual bool			Create(uint x, uint y, uint width, uint height, uint depth, bool fullscreen) = 0;	//! Creates the window
	virtual void			Destroy() = 0;									//!< Destroys the window

	virtual bool			IsMain() const;									//!< Returns whether is the main
	virtual bool			IsVisible() const;								//!< Returns whether is visible
	virtual void			SetVisible(bool visible) = 0;					//!< Sets whether is visible

	virtual void			GetPosition(uint& x, uint& y);					//!< Returns the position
	virtual void			Move(uint x, uint y) = 0;						//!< Sets the position

	virtual void			GetMetrics(uint& width, uint& height) const;	//!< Returns width and height
	virtual void			Resize(uint width, uint height) = 0;			//!< Sets width and height

	virtual bool			IsFullscreen() const;							//!< Returns whether in fullscreen
	virtual void			SetFullscreen(bool full) = 0;					//!< Switches to fullscreen

	const Str&				GetTitle() const;								//!< Returns the title
	void					SetTitle(const Str& title);						//!< Sets the title
	const Str&				GetAdditionalTitleText() const;					//!< Get the additional title text
	void					SetAdditionalTitleText(const Str& title);		//!< Set the additional title text
	Str						GetDisplayedTitle() const;						//!< Returns the actual title

	virtual void			Update() = 0;									//!< Updates displayed content
	virtual void			EventLoop() = 0;								//!< Manages window events

protected:

	void					_SetMain(bool main);							//!< Sets as the main window, only Device should use it

	uint					mX, mY;
	uint					mWidth, mHeight;
	bool					mVisible;
	bool					mActive;
	bool					mFull;
	bool					mCreated;
	bool					mMain;
	Str						mTitle;
	Str						mAdditionalTitleText;

	virtual void	_SetTitleAndAdditionalTextToWindow() = 0;		//!< Sets the title and the additional title text

	friend class	Device;
};

} // namespace Crown

