package crown.android;

import java.util.List;
import java.lang.Math;

import android.content.Context;
import android.util.Log;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;

import crown.android.CrownEnum;


/**
*	CrownAccelerometer manage Accelerometer sensor
*	for Android Devices.
*/
public class CrownAccelerometer
{
    private static final float MIN_VALUE = 0.0f;
    private static final float MAX_VALUE = 1.0f;

    private static SensorManager sensorManager;
    private static Sensor sensor;
    private static SensorEventListener sensorEventListener;

    private static float mX;
    private static float mY;
    private static float mZ; 
    private static float lastX;
    private static float lastY;
    private static float lastZ;

//-----------------------------------------------------------------------------------
    private static boolean initAccelerometer(Context context)
    {
    	// already initialized!
    	// if (sensorEventListener != null)
    	// {
    	//     return true;
    	// }

    	sensorManager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);

    	sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

    	sensorEventListener = new SensorEventListener()
    	{
	    	@Override
	    	public void onAccuracyChanged(Sensor sensor, int accuracy) 
			{
	    	}

	    	@Override
	    	public void onSensorChanged(SensorEvent event) 
			{
                mX = event.values[0];
                mY = event.values[1];
                mZ = event.values[2];

				CrownLib.pushFloatEvent(CrownEnum.OSET_ACCELEROMETER, mX, mY, mZ, 0);

                lastX = mX;
                lastY = mY;
                lastZ = mZ;
    		}    		
    	};

		sensorManager.registerListener(sensorEventListener, sensor, SensorManager.SENSOR_DELAY_GAME);
		return true;
    }

//-----------------------------------------------------------------------------------
    public static boolean startListening(Context context)
    {
    	return initAccelerometer(context); 
    }

//-----------------------------------------------------------------------------------
    public static void stopListening()
    {
    	sensorManager.unregisterListener(sensorEventListener);
    }

//-----------------------------------------------------------------------------------
    public static float getX()
    {
    	return mX;
    }

//-----------------------------------------------------------------------------------
    public static float getY()
    {
    	return mY;
    }

//-----------------------------------------------------------------------------------
    public static float getZ()
    {
    	return mZ;
    }

//-----------------------------------------------------------------------------------
    public static float getLastX()
    {
    	return lastX;
    }

//-----------------------------------------------------------------------------------
    public static float getLastY()
    {
    	return lastY;
    }

//-----------------------------------------------------------------------------------
    public static float getLastZ()
    {
    	return lastZ;
    }

// //-----------------------------------------------------------------------------------
//     private void lowPassFiltering(float uX, float uY, float uZ)
//     {
//         float updateFreq = 30; // match this to your update speed
//         float cutOffFreq = 0.9f;
//         float timeRC = 1.0f / cutOffFreq;
//         float dt = 1.0f / updateFreq;
//         float filterConstant = timeRC / (dt + timeRC);
//         float alpha = filterConstant;                 
//         float kAccelerometerMinStep = 0.033f;
//         float kAccelerometerNoiseAttenuation = 3.0f;

//         float d = clamp((Math.abs(norm(mX, mY, mZ) - norm(uX, uY, uZ)) / kAccelerometerMinStep - 1.0f), MIN_VALUE, MAX_VALUE);
//         alpha = d * filterConstant / kAccelerometerNoiseAttenuation + (1.0f - d) * filterConstant;

//         mX = (float) (alpha * (mX + uX - lastX));
//         mY = (float) (alpha * (mY + uY - lastY));
//         mZ = (float) (alpha * (mZ + uZ - lastZ));

//         lastX = mX;
//         lastY = mY;
//         lastZ = mZ;
//     }

// //-----------------------------------------------------------------------------------
//     private float clamp(float v, float min, float max)
//     {
//         if (v <= min)
//         {
//             v = min;
//         }
//         else if (v >= max)
//         {
//             v = max;
//         }

//         return v;
//     }

// //-----------------------------------------------------------------------------------
//     private float norm(float x, float y, float z)
//     {
//         float v = (float)Math.pow(x, 2) + (float)Math.pow(y, 2) + (float)Math.pow(z, 2);
//         return (float)Math.sqrt(v);
//     }
}