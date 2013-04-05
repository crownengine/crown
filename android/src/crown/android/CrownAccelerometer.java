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
    private static final float MIN_VALUE = -1.0f;
    private static final float MAX_VALUE = 1.0f;

    private static SensorManager sensorManager;
    private static Sensor sensor;
    private static SensorEventListener sensorEventListener;
    private static float x;
    private static float y;
    private static float z; 
    private static float lastX;
    private static float lastY;
    private static float lastZ;

    private float lastAccel[] = new float[3];
    private float accelFilter[] = new float[3];

//-----------------------------------------------------------------------------------
    private static boolean initAccelerometer(Context context)
    {
    	// already initialized!
    	// if (sensorEventListener != null)
    	// {
    	// 	return true;
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
                lowPassFiltering(event.values[0], event.values[1], event.values[2]);

				CrownLib.pushFloatEvent(CrownEnum.OSET_ACCELEROMETER, x, y, z, 0);

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
    	return x;
    }

//-----------------------------------------------------------------------------------
    public static float getY()
    {
    	return y;
    }

//-----------------------------------------------------------------------------------
    public static float getZ()
    {
    	return z;
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

//-----------------------------------------------------------------------------------
    private static void lowPassFiltering(float uX, float uY, float uZ)
    {
        float updateFreq = 30; // match this to your update speed
        float cutOffFreq = 50.0f;
        float timeRC = 1.0f / cutOffFreq;
        float dt = 1.0f / updateFreq;
        float filterConstant = timeRC / (dt + timeRC);
        float alpha = filterConstant; 
        float kAccelerometerMinStep = 0.033f;
        float kAccelerometerNoiseAttenuation = 3.0f;

        float d = clamp(Math.abs(norm(x, y, z) - norm(uX, uY, uZ)) / kAccelerometerMinStep - 1.0f, MIN_VALUE, MAX_VALUE);
        alpha = d * filterConstant / kAccelerometerNoiseAttenuation + (1.0f - d) * filterConstant;

        x = (float) (alpha * (x + uX - lastX));
        y = (float) (alpha * (y + uY - lastY));
        z = (float) (alpha * (z + uZ - lastZ));


        lastX = x;
        lastY = y;
        lastZ = z;
    }

//-----------------------------------------------------------------------------------
    private static float clamp(float v, float min, float max)
    {
        if (v < min)
        {
            v = min;
        }
        else if (v > max)
        {
            v = max;
        }

        return v;
    }

//-----------------------------------------------------------------------------------
    private static float norm(float x, float y, float z)
    {
        float v = (float)Math.pow(x, 2) + (float)Math.pow(y, 2) + (float)Math.pow(z, 2);
        return (float)Math.sqrt(v);
    }
}