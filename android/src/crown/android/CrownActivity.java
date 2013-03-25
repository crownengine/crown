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
	// debug
	public static String TAG = "CrownActivity";

	// Resource attributes
    static AssetManager assetManager;

	// Graphic attributes
	private CrownView mView;

	// Input attributes
	private static SensorManager sm;
	private Sensor sensor;

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
		// init SensorManager -> accelerometer
	    sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
	    sensor = sm.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
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
		if (sensor != null) 
		{
		  sm.registerListener(sensorEventListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
		  Log.i(TAG, "ACCELEROMETER sensor registered.");
		} 
		else 
		{
		  Log.e(TAG, "ACCELEROMETER sensor cannot be registered. The application will be terminated.");
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
		// dismise accelerometer
		sm.unregisterListener(sensorEventListener);
	}

	/**
	*
	*/
	public void onDestroy()
	{
		super.onDestroy();
		Log.i(TAG, "onDestroy called.");

		//CrownLib.shutdown();
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
//				Log.i(TAG, "event: ACTION_DOWN_" + pointerId + ", x=" + x + " y=" + y);
				CrownLib.pushEvent(CrownEnum.OSET_TOUCH_DOWN, pointerId, (int) x,(int) y, 0);
				break;			
			}

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			{
//				Log.i(TAG, "event = ACTION_UP_" + pointerId + ", x=" + x + " y=" + y);
				CrownLib.pushEvent(CrownEnum.OSET_TOUCH_UP, pointerId, (int) x,(int) y, 0);
				break;			
			}
			
			case MotionEvent.ACTION_MOVE:
			{
				for (int index = 0; index < pointerCount; index++)
				{
//					Log.i(TAG, "event = ACTION_MOVE_" + event.getPointerId(index) + " X=" + (int)event.getX(index) + " Y=" + (int)event.getY(index));
					CrownLib.pushEvent(CrownEnum.OSET_TOUCH_MOVE, event.getPointerId(index), (int)event.getX(index),(int)event.getY(index), 0);
				}

				break;
			}
		}

		return true;
	}

	/**
	*	Listener which embed callback methods for
	*	accelerometer data
	*/
  	private SensorEventListener sensorEventListener = new SensorEventListener() 
	{
    	@Override
    	public void onAccuracyChanged(Sensor sensor, int accuracy) 
		{
    	}

    	@Override
    	public void onSensorChanged(SensorEvent event) 
		{
     	 	float x = event.values[0];
			float y = event.values[1];
			float z = event.values[2];

//			Log.i(TAG, "X:" + x + "Y:" + y + "Z:" + z);
			CrownLib.pushEvent(11,(int) x,(int) y, (int)z, 0);
    	}
  	};
}
