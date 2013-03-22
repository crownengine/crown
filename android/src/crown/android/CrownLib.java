package crown.android;

public class CrownLib
{
	static 
	{
		System.loadLibrary("crown");
	}

	public static native void pushEvent(int type, int a, int b, int c, int d);
}
