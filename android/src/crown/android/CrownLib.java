package crown.android;

import android.content.res.AssetManager;

public class CrownLib
{
	static 
	{
		System.loadLibrary("crown");
		System.loadLibrary("triangle");
	}
	
//	// Device functions
	public static native void init();
	public static native void frame();
	public static native void shutdown();

	// AssetManager functions
	public static native void initAssetManager(AssetManager assetManager);

	// InputManager functions
	public static native void pushEvent(int type, int a, int b, int c, int d);


}
