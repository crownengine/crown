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
    static AssetManager assetManager;

	// Graphic attributes
	private CrownView mView;

	/**
	*
	*/
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		// init AssetManager
		assetManager = getAssets();
		CrownLib.initAssetManager(assetManager);

        // Init GLSurfaceView for rendering
        mView = new CrownView(getApplication());
		setContentView(mView);

		Log.i(TAG, "onCreate called.");
    }

	/**
	*
	*/
	public void onStart()
	{
		super.onStart();
		Log.i(TAG, "onStart called.");
	}

	/**
	*
	*/
	public void onRestart()
	{
		super.onRestart();
		Log.i(TAG, "onRestart called.");
	}

	/**
	*
	*/
	public void onResume()
	{
		super.onResume();
        mView.onResume();
		
		Log.i(TAG, "onResume called.");

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
		Log.i(TAG, "onPause called.");
	}

	/**
	*
	*/
	public void onStop()
	{
		super.onStop();
		Log.i(TAG, "onStop called.");

		// stop accelerometer
		CrownAccelerometer.stopListening();
	}

	/**
	*
	*/
	public void onDestroy()
	{
		super.onDestroy();
		Log.i(TAG, "onDestroy called.");
	}

	/**
	*	Callback method which takes touch data and 
	*	sends them to Crown.
	*/
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
				CrownLib.pushIntEvent(CrownEnum.OSET_TOUCH_DOWN, pointerId, (int)x, (int)y, 0);
				break;			
			}

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			{
				CrownLib.pushIntEvent(CrownEnum.OSET_TOUCH_UP, pointerId, (int)x, (int)y, 0);
				break;			
			}
			
			case MotionEvent.ACTION_MOVE:
			{
				for (int index = 0; index < pointerCount; index++)
				{
					CrownLib.pushIntEvent(CrownEnum.OSET_TOUCH_MOVE, event.getPointerId(index), (int)event.getX(index), (int)event.getY(index), 0);
				}

				break;
			}
		}

		return true;
	}
}
