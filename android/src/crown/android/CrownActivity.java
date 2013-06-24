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
import android.view.Surface;
import android.view.SurfaceView;

import crown.android.CrownEnum;

/**
*	BootStrap of Android Application
*/
public class CrownActivity extends Activity
{

	// Debug
	public static String TAG = "CrownActivity";

	// Resource attributes
    static AssetManager 		mAssetManager;

	// Input attributes
	private CrownTouch 			mTouch;
	private CrownSensor			mSensor;

	// Graphic attributes
	static SurfaceView			mWindow;

//-----------------------------------------------------------------------------------
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		// init AssetManager
		mAssetManager = getAssets();
		CrownLib.initAssetManager(mAssetManager);

		// Init Input
		mTouch = new CrownTouch(this);
		mSensor = new CrownSensor(this);
		
        mWindow = (SurfaceView)findViewById(R.id.crown_window);
        CrownLib.setWindow(mWindow);
    }

//-----------------------------------------------------------------------------------
	public void onStart()
	{
		super.onStart();
	}

//-----------------------------------------------------------------------------------
	public void onRestart()
	{
		super.onRestart();
	}

//-----------------------------------------------------------------------------------
	public void onResume()
	{
		super.onResume();
		
		// init accelerometer
		if (!mSensor.startListening(this))
		{
			finish();
		}
	}

//-----------------------------------------------------------------------------------
	public void onPause()
	{
		super.onPause();
	}

//-----------------------------------------------------------------------------------
	public void onStop()
	{
		super.onStop();

		// stop accelerometer
		mSensor.stopListening();
	}

//-----------------------------------------------------------------------------------
	public void onDestroy()
	{
		super.onDestroy();
	}

//-----------------------------------------------------------------------------------
	public boolean onTouchEvent(MotionEvent event)
	{
		mTouch.onTouch(event);
        return super.onTouchEvent(event);
	}

//-----------------------------------------------------------------------------------
	public boolean hasMultiTouchSupport(Context context)
	{
		return context.getPackageManager().hasSystemFeature("android.hardware.touchscreen.multitouch");
	}
}