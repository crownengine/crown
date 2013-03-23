package crown.android;

import android.content.res.AssetManager;

public class CrownLib
{
	static 
	{
		System.loadLibrary("crown");
	}

	public static native void initAssetManager(AssetManager assetManager);

	public static native void pushEvent(int type, int a, int b, int c, int d);
}
