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
*	CrownSensor manage sensors for Android Devices.
*/
public class CrownSensor
{
    private final float MIN_VALUE = -1.0f;
    private final float MAX_VALUE = 1.0f;
    private final static float RAD2DEG = (float) (180.0f / Math.PI);

    private SensorManager sensorManager;
    private Sensor mAccelerometerSensor;
    private Sensor mCompassSensor;
    private SensorEventListener mAccelerometerEventListener;
    private SensorEventListener mCompassEventListener;
    private boolean isAccelerometerAvailable;
    private boolean isCompassAvailable;

    private float[] mRotationMatrix;
    private float[] mGravity;
    private float[] mLastGravity;
    private float[] mBufferedAccelGData;
    private float[] mGeoMagn;
    private float[] mBufferedMagnetData;
    private float[] mOrientation;

    private float[] mRotAngle;
 

//-----------------------------------------------------------------------------------
    public CrownSensor(Context context)
    {
        mRotationMatrix = new float[16];
        mGravity = new float[3];
        mLastGravity = new float[3];
        mGeoMagn = new float[3];
        mOrientation = new float[3];
        mRotAngle = new float[3];

        isAccelerometerAvailable = context.getPackageManager().hasSystemFeature("android.hardware.sensor.accelerometer");
        isCompassAvailable = context.getPackageManager().hasSystemFeature("android.hardware.sensor.compass");

        if (hasAccelerometerSupport())
        {
            mAccelerometerEventListener = new SensorEventListener()
            {
                public void onAccuracyChanged(Sensor sensor, int accuracy) 
                {
                }

                public void onSensorChanged(SensorEvent event)
                {
                    mGravity[0] = (mGravity[0] * 2 + event.values[0]) * 0.33334f * RAD2DEG;
                    mGravity[1] = (mGravity[1] * 2 + event.values[1]) * 0.33334f * RAD2DEG;
                    mGravity[2] = (mGravity[2] * 2 + event.values[2]) * 0.33334f * RAD2DEG;

                    norm();

                    CrownLib.pushFloatEvent(CrownEnum.OSET_ACCELEROMETER, mGravity[0], mGravity[1], mGravity[2], 0.0f);
                }
            };           
        }

        if (hasCompassSupport())
        {
            mCompassEventListener = new SensorEventListener()
            {
                public void onAccuracyChanged(Sensor sensor, int accuracy) 
                {
                }

                public void onSensorChanged(SensorEvent event)
                {
                    mGeoMagn[0] = (mGeoMagn[0] + event.values[0]) * 0.5f;
                    mGeoMagn[1] = (mGeoMagn[1] + event.values[1]) * 0.5f;
                    mGeoMagn[2] = (mGeoMagn[2] + event.values[2]) * 0.5f; 

                    // CrownLib.pushFloatEvent(CrownEnum.OSET_ACCELEROMETER, mGeoMagn[0], mGeoMagn[1], mGeoMagn[2], 0.0f);
                }                
            };
        }
    }

//-----------------------------------------------------------------------------------
    public boolean startListening(Context context)
    {
        sensorManager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);

        mAccelerometerSensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        mCompassSensor = sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);

        sensorManager.registerListener(mAccelerometerEventListener, mAccelerometerSensor, SensorManager.SENSOR_DELAY_GAME);
        sensorManager.registerListener(mCompassEventListener, mCompassSensor, SensorManager.SENSOR_DELAY_GAME);

        return true;
    }

//-----------------------------------------------------------------------------------
    public void stopListening()
    {
    	sensorManager.unregisterListener(mAccelerometerEventListener);
        sensorManager.unregisterListener(mCompassEventListener);
    }

//-----------------------------------------------------------------------------------
    public boolean hasAccelerometerSupport()
    {
        return isAccelerometerAvailable;       
    }

//-----------------------------------------------------------------------------------
    public boolean hasCompassSupport()
    {   
        return isCompassAvailable;
    }

 //-----------------------------------------------------------------------------------
//     private void lowPassFiltering(float x, float y, float z)
//     {
//         float updateFreq = 30; // match this to your update speed
//         float cutOffFreq = 0.9f;
//         float timeRC = 1.0f / cutOffFreq;
//         float dt = 1.0f / updateFreq;
//         float filterConstant = timeRC / (dt + timeRC);
//         float alpha = filterConstant;                 
//         float kAccelerometerMinStep = 0.033f;
//         float kAccelerometerNoiseAttenuation = 3.0f;

//         float d = clamp((Math.abs(norm(mGravity[0], mGravity[1], mGravity[2]) - norm(x, y, z)) / kAccelerometerMinStep - 1.0f), MIN_VALUE, MAX_VALUE);
//         alpha = d * filterConstant / kAccelerometerNoiseAttenuation + (1.0f - d) * filterConstant;

//         mGravity[0] = (float) (alpha * (mGravity[0] + x - mLastGravity[0]));
//         mGravity[1] = (float) (alpha * (mGravity[1] + y - mLastGravity[1]));
//         mGravity[2] = (float) (alpha * (mGravity[2] + z - mLastGravity[2]));

//         mLastGravity[0] = mGravity[0];
//         mLastGravity[1] = mGravity[1];
//         mLastGravity[2] = mGravity[2];
//     }

//-----------------------------------------------------------------------------------
    private float clamp(float v, float min, float max)
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
    private void norm()
    {
        float v = mGravity[0] * mGravity[0] + mGravity[1] * mGravity[1] + mGravity[2] * mGravity[2];
        float magnitude = (float)Math.sqrt(v);
        float invMagnitude = 1 / magnitude;

        mGravity[0] *= invMagnitude;
        mGravity[1] *= invMagnitude;
        mGravity[2] *= invMagnitude;

        mGravity[0] = clamp(mGravity[0], MIN_VALUE, MAX_VALUE);
        mGravity[1] = clamp(mGravity[1], MIN_VALUE, MAX_VALUE);
        mGravity[2] = clamp(mGravity[2], MIN_VALUE, MAX_VALUE);
    }
}