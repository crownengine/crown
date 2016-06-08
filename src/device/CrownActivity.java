/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

package crown.android;

import android.app.NativeActivity;
import android.os.Bundle;

public class CrownActivity extends NativeActivity
{
	static
	{
		System.loadLibrary("crown");
	}

	CrownActivity _activity;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		_activity = this;
		// Init additional stuff here (ads, etc.)
	}

	@Override
	public void onDestroy()
	{
		// Destroy additional stuff here (ads, etc)
		super.onDestroy();
	}
}
