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

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.view.MotionEvent;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.content.Context;
import android.widget.Toast;
import android.content.res.AssetManager;
import android.view.View;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;
import android.view.KeyEvent;

public class CrownActivity extends Activity
{
	// Debug
	public static String TAG = "crown";

	// Resource attributes
    static AssetManager 		mAssetManager;
	private CrownSurfaceView 	mView;


	//-----------------------------------------------------------------------------
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        // Initializes low-level systems (memory, os etc.)
        CrownLib.initCrown();

		// init AssetManager
		mAssetManager = getAssets();
		CrownLib.initAssetManager(mAssetManager);

		// init Native Window
		mView = new CrownSurfaceView(this);
        setContentView(mView);

		Log.i(TAG, "Crown Activity created");
    }

	//-----------------------------------------------------------------------------
	public void onResume()
	{
		super.onResume();
		CrownLib.pushResumeEvent();
		Log.i(TAG, "Crown Activity resumed");
	}

	//-----------------------------------------------------------------------------
	public void onPause()
	{
		super.onPause();
		CrownLib.pushPauseEvent();
		Log.i(TAG, "Crown Activity paused");
	}

	//-----------------------------------------------------------------------------
	public void onDestroy()
	{
		super.onDestroy();
		CrownLib.pushExitEvent(0);
		CrownLib.releaseWindow();
		CrownLib.shutdownCrown();
		Log.i(TAG, "Crown Activity destroyed");
	}

	//-----------------------------------------------------------------------------
	public void onBackPressed()
	{
		// Simulate ESCAPE key
		CrownLib.pushKeyboardEvent(0, 0x1B, 1);
	}

	//-----------------------------------------------------------------------------
	public boolean onTouchEvent(MotionEvent event)
	{
		final int pointerIndex = event.getActionIndex();
		final int pointerCount = event.getPointerCount();

		final int pointerId = event.getPointerId(pointerIndex);
		final float x = event.getX(pointerIndex);
		final float y = event.getY(pointerIndex);

		final int actionMasked = event.getActionMasked();

		switch (actionMasked) 
		{	
			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_POINTER_DOWN:
			{
				CrownLib.pushTouchEventPointer(pointerId, (int) x, (int) y, 1);
				break;			
			}
			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			{
				CrownLib.pushTouchEventPointer(pointerId, (int) x, (int) y, 0);
				break;
			}
			case MotionEvent.ACTION_OUTSIDE:
			case MotionEvent.ACTION_CANCEL:
			{
				CrownLib.pushTouchEventPointer(pointerId, (int)x, (int)y, 0);
				break;			
			}
			case MotionEvent.ACTION_MOVE:
			{
				for (int index = 0; index < pointerCount; index++)
				{
					CrownLib.pushTouchEventMove(event.getPointerId(index), (int)event.getX(index), (int)event.getY(index));
				}
				break;
			}
		}

        return super.onTouchEvent(event);
	}
}
