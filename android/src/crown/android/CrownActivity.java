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
import android.view.ScaleGestureDetector;

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

	// Graphic attributes
	private CrownView 			mView;

	// Input attributes
	private CrownTouch 			mTouch;
	private CrownSensor			mSensor;

	// Gestures detectors
	private ScaleGestureDetector mScaleDetector;

//-----------------------------------------------------------------------------------
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		// init AssetManager
		mAssetManager = getAssets();
		CrownLib.initAssetManager(mAssetManager);

        // Init GLSurfaceView for rendering
        mView = new CrownView(getApplication());
		setContentView(mView);

		// Init Input
		mTouch = new CrownTouch(this);
		mSensor = new CrownSensor(this);
		
	    mScaleDetector = new ScaleGestureDetector(this, new ScaleListener());
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
        mView.onResume();
		
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
        mView.onPause();
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
		mScaleDetector.onTouchEvent(event);

		mTouch.onTouch(event);
        return super.onTouchEvent(event);
	}

//-----------------------------------------------------------------------------------
	public boolean hasMultiTouchSupport(Context context)
	{
		return context.getPackageManager().hasSystemFeature("android.hardware.touchscreen.multitouch");
	}


//-----------------------------------------------------------------------------------
	private class ScaleListener extends ScaleGestureDetector.SimpleOnScaleGestureListener
	{
	    public boolean onScale(ScaleGestureDetector detector) 
	    {
	    	Log.i(TAG, "onScale.");
	    	return true;
	    }

	    public boolean onScaleBegin(ScaleGestureDetector detector) 
	    {
	    	Log.i(TAG, "onScaleBegin.");
	    	return true;
	    }

	    public void onScaleEnd(ScaleGestureDetector detector) 
	    {
	    	Log.i(TAG, "onScaleBegin.");
	    }
	};
}