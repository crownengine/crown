package crown.android;

import android.content.res.AssetManager;

public class CrownLib
{
	static 
	{
		System.loadLibrary("crown");
	}
	
	// Device functions
	public static native void init();
	public static native void frame();
	public static native void shutdown();

	// AssetManager functions
	public static native void initAssetManager(AssetManager assetManager);

	// InputManager functions
	public static native void pushIntEvent(int type, int a, int b, int c, int d);
	public static native void pushFloatEvent(int type, float a, float b, float c, float d);

	// RenderWindow functions
	public static native void setRenderWindowMetrics(int width, int height);

}
