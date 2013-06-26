package crown.android;

import android.content.Context;
import android.view.Surface;
import android.view.SurfaceView;
import android.view.SurfaceHolder;

public class CrownSurfaceView extends SurfaceView implements SurfaceHolder.Callback
{
//-----------------------------------------------------------------------------
	public CrownSurfaceView(Context context)
	{
		super(context);
		this.getHolder().addCallback(this);
	}

//-----------------------------------------------------------------------------
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) 
    {    
    	if (CrownLib.isRunning())
    	{
    		CrownLib.frame();
    	}
    }

//-----------------------------------------------------------------------------
    @Override
    public void surfaceCreated(SurfaceHolder holder) 
    {
        CrownLib.setWindow(holder.getSurface());
    	CrownLib.init();
    }

//-----------------------------------------------------------------------------
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) 
    {  
    	if (CrownLib.isInit())
    	{
    		CrownLib.shutdown();
    	}
    }
}