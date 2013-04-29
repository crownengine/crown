package crown.android;

import android.content.Context;
import android.util.Log;
import android.view.MotionEvent;


/**
* CrownTouch manages touch and gesture events passing them to Crown Engine
*/

//TODO: gestures doesn't work...fix them
public class CrownTouch
{
	private boolean 				isListening;

//-----------------------------------------------------------------------------------
	public CrownTouch(Context context)
	{
		isListening = false;
	}

//-----------------------------------------------------------------------------------
	public void onTouch(MotionEvent event)
	{
		final int pointerIndex = event.getActionIndex();
		final int pointerCount = event.getPointerCount();

		final int pointerId = event.getPointerId(pointerIndex);
		final float x = event.getX(pointerIndex);
		final float y = event.getY(pointerIndex);

		final int actionMasked = event.getActionMasked();

		switch (actionMasked) 
		{	
			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_POINTER_DOWN:
			{
				CrownLib.pushIntEvent(CrownEnum.OSET_TOUCH_DOWN, pointerId, (int)x, (int)y, 0);
				break;			
			}

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			case MotionEvent.ACTION_OUTSIDE:
			case MotionEvent.ACTION_CANCEL:
			{
				CrownLib.pushIntEvent(CrownEnum.OSET_TOUCH_UP, pointerId, (int)x, (int)y, 0);
				break;			
			}
			
			case MotionEvent.ACTION_MOVE:
			{
				for (int index = 0; index < pointerCount; index++)
				{
					CrownLib.pushIntEvent(CrownEnum.OSET_TOUCH_MOVE, event.getPointerId(index), (int)event.getX(index), (int)event.getY(index), 0);
				}

				break;
			}
		}
	}
}