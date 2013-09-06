/*
Copyright (c) 2013 Daniele Bartolini, Michele Rossi
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

package crown.android;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

import android.util.Log;

public class CrownSurfaceView extends SurfaceView implements SurfaceHolder.Callback
{
	private final String TAG = "crown";

	//-----------------------------------------------------------------------------
	public CrownSurfaceView(Context context)
	{
		super(context);

		getHolder().addCallback(this);

		setFocusable(true);
	}

	//-----------------------------------------------------------------------------
	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) 
	{
		Log.d(TAG, "Crown Surface changed");
	}

	//-----------------------------------------------------------------------------
	@Override
	public void surfaceCreated(SurfaceHolder holder) 
	{
		Log.d(TAG, "Crown Surface created");

		CrownLib.setWindow(getHolder().getSurface());

		if (!CrownLib.isDeviceInit())
		{
			CrownLib.initDevice();
		}
		else
		{
			CrownLib.unpauseDevice();
		}
	}

	//-----------------------------------------------------------------------------
	@Override
	public void surfaceDestroyed(SurfaceHolder holder) 
	{
		Log.d(TAG, "Crown Surface destroyed");
		
		CrownLib.pauseDevice();
		CrownLib.invalidateRenderer();
	}
}
