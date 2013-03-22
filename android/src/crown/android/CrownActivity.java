package crown.android;

import android.app.Activity;
import android.os.Bundle;
import android.widget.Toast;

public class CrownActivity extends Activity
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
		CrownLib.create();
		Toast.makeText(this, "EGL window created", Toast.LENGTH_SHORT).show();
    }

	public void onStart()
	{
		super.onStart();
	}
	
	public void onRestart()
	{
		super.onRestart();
	}

	public void onResume()
	{
		super.onResume();
	}

	public void onPause()
	{
		super.onPause();
	}

	public void onStop()
	{
		super.onStop();
	}

	public void onDestroy()
	{
		super.onDestroy();
		CrownLib.destroy();
		Toast.makeText(this, "EGL window destroyed", Toast.LENGTH_SHORT).show();
	}
}
