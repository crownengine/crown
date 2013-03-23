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

public class CrownActivity extends Activity
{
	public static String TAG = "CrownActivity";

	// Resource attributes
    static AssetManager assetManager;

	// Graphic attributes
	private CrownView mView;

	// Input attributes
	private static SensorManager sm;
	private Sensor sensor;
	private int mActivePointer = -1;

//---------------------------------------------------------------------
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        
        setContentView(R.layout.main);

		assetManager = getAssets();
		CrownLib.initAssetManager(assetManager);
        
        mView = new CrownView(getApplication());
	    sm = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
	    sensor = sm.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
    }

//---------------------------------------------------------------------
	public void onStart()
	{
		super.onStart();
	}

//---------------------------------------------------------------------	
	public void onRestart()
	{
		super.onRestart();
	}

//---------------------------------------------------------------------
	public void onResume()
	{
		super.onResume();
        mView.onResume();

		if (sensor != null) 
		{
		  sm.registerListener(sensorEventListener, sensor, SensorManager.SENSOR_DELAY_NORMAL);
		  Log.i(TAG, "Registerered for ORIENTATION Sensor");

		} 
		else 
		{
		  Log.e("Compass MainActivity", "Registerered for ORIENTATION Sensor");
		  Toast.makeText(this, "ORIENTATION Sensor not found", Toast.LENGTH_LONG).show();
		  finish();
		}
	}

//---------------------------------------------------------------------
	public void onPause()
	{
		super.onPause();
        mView.onPause();

		sm.unregisterListener(sensorEventListener);
	}

//---------------------------------------------------------------------
	public void onStop()
	{
		super.onStop();
	}

//---------------------------------------------------------------------
	public void onDestroy()
	{
		super.onDestroy();
	}

//---------------------------------------------------------------------
	public boolean onTouchEvent(MotionEvent event)
	{

		int pointerIndex = event.getActionIndex();
		int pointerCount = event.getPointerCount();

		int pointerId = event.getPointerId(pointerIndex);
		float x = event.getX(pointerIndex);
		float y = event.getY(pointerIndex);

		int actionMasked = event.getActionMasked();

		switch (actionMasked) 
		{	
			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_POINTER_DOWN:
			{
				Log.i(TAG, "event = ACTION_DOWN_" + pointerId);
				CrownLib.pushEvent(6, pointerId, (int) x,(int) y, 0);
				break;			
			}

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			{
				Log.i(TAG, "event = ACTION_UP_" + pointerId);
				CrownLib.pushEvent(7, pointerId, (int) x,(int) y, 0);
				break;			
			}
			
			case MotionEvent.ACTION_MOVE:
			{
				for (int index = 0; index < pointerCount; index++)
				{
					//Log.i(TAG, "event = ACTION_MOVE_" + event.getPointerId(index) + " X=" + (int)event.getX(index) + " Y=" + (int)event.getY(index));
					CrownLib.pushEvent(8, event.getPointerId(index), (int)event.getX(index),(int)event.getY(index), 0);
				}

				break;
			}
		}

		return true;
	}

//---------------------------------------------------------------------
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
