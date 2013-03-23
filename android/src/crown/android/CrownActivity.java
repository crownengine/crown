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


public class CrownActivity extends Activity
{
	public static String TAG = "CrownActivity";

	private CrownView mView;

	private static SensorManager sm;
	private Sensor sensor;

	//TODO: add count of finger on screen at the same time

//---------------------------------------------------------------------
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
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
	    float x = event.getX();
	    float y = event.getY();

		switch (event.getAction()) 
		{	
	        case MotionEvent.ACTION_MOVE:
			{
				Log.i(TAG, "event = ACTION_MOVE");
				CrownLib.pushEvent(10,(int) x,(int) y, 0, 0);
				break;
			}

			case MotionEvent.ACTION_DOWN:
			{
				Log.i(TAG, "event = ACTION_DOWN");
				CrownLib.pushEvent(10,(int) x,(int) y, 0, 0);
				break;			
			}

			case MotionEvent.ACTION_UP:
			{
				Log.i(TAG, "event = ACTION_UP");
				CrownLib.pushEvent(10,(int) x,(int) y, 0, 0);
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
			Log.i(TAG, "X:" + x + "Y:" + y + "Z:" + z);
			CrownLib.pushEvent(11,(int) x,(int) y, (int)z, 0);
    	}
  	};
}
