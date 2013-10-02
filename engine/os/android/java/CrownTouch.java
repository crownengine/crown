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

import android.content.Context;
import android.util.Log;
import android.view.MotionEvent;


/**
* CrownTouch manages touch and gesture events passing them to Crown Engine
*/

public class CrownTouch
{
	private boolean isListening;

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
				CrownLib.pushTouchEvent(CrownEnum.OSET_TOUCH_DOWN, pointerId, (int)x, (int)y);
				break;			
			}

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
			case MotionEvent.ACTION_OUTSIDE:
			case MotionEvent.ACTION_CANCEL:
			{
				CrownLib.pushTouchEvent(CrownEnum.OSET_TOUCH_UP, pointerId, (int)x, (int)y);
				break;			
			}
			
			case MotionEvent.ACTION_MOVE:
			{
				for (int index = 0; index < pointerCount; index++)
				{
					CrownLib.pushTouchEvent(CrownEnum.OSET_TOUCH_MOVE, event.getPointerId(index), (int)event.getX(index), (int)event.getY(index));
				}

				break;
			}
		}
	}
}