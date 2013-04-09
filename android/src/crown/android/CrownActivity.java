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
	private CrownTouch 			mTouchListener;

	/**
	*
	*/
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
		mTouchListener = new CrownTouch();
    }

	/**
	*
	*/
	public void onStart()
	{
		super.onStart();
	}

	/**
	*
	*/
	public void onRestart()
	{
		super.onRestart();
	}

	/**
	*
	*/
	public void onResume()
	{
		super.onResume();
        mView.onResume();
		
		// init accelerometer
		if (!CrownAccelerometer.startListening(this))
		{
			Log.i(TAG, "Device has no accelerometer. App terminated.");
			finish();
		}
	}

	/**
	*
	*/
	public void onPause()
	{
		super.onPause();
        mView.onPause();
	}

	/**
	*
	*/
	public void onStop()
	{
		super.onStop();

		// stop accelerometer
		CrownAccelerometer.stopListening();
	}

	/**
	*
	*/
	public void onDestroy()
	{
		super.onDestroy();
	}

	/**
	*	Callback method which takes touch data and 
	*	sends them to Crown.
	*/
	public boolean onTouchEvent(MotionEvent event)
	{
		mTouchListener.onTouch(event);
		return true;
	}
}
