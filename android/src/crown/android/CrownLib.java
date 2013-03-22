package crown.android;

public class CrownLib
{
	static 
	{
		System.loadLibrary("crown");
	}

	public static native boolean create();
	public static native void destroy();
}
