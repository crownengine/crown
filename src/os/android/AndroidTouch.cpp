/*
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

#include "AndroidTouch.h"
#include <android/input.h>
#include "Log.h"
#include "AndroidInputManager.h"

namespace Crown
{

AndroidTouch::AndroidTouch(InputManager* creator) :
	Touch(creator),
	mAndroidApp(NULL)
{
	if (mCreator)
	{
		mAndroidApp = static_cast<AndroidInputManager*>(mCreator)->GetAndroidApp();
		static_cast<AndroidInputManager*>(mCreator)->SetTouchAvailable(true);
	}
}

AndroidTouch::~AndroidTouch()
{
	if (mCreator)
	{
		static_cast<AndroidInputManager*>(mCreator)->SetTouchAvailable(false);
	}
}

void AndroidTouch::EventLoop()
{
	int ident;
	int events;
	struct android_poll_source* source;

	struct android_app* app = mAndroidApp;

	if ((ident=ALooper_pollOnce(0, NULL, &events, (void**)&source)) >= 0)
	{
		if (source != NULL && source->id == LOOPER_ID_INPUT)
		{
			AInputEvent* event = NULL;

			if (AInputQueue_getEvent(app->inputQueue, &event) >= 0)
			{
				if (AInputQueue_preDispatchEvent(app->inputQueue, event))
				{
					return;
				}

				int32_t handled = 0;
				handled = this->HandleTouchEvent(app, event);
				AInputQueue_finishEvent(app->inputQueue, event, handled);
			}
			else
			{
				//LOGI("Failure reading next input event: %s\n", strerror(errno));
			}
			//LOGI("Uscito da LOOPER_ID_INPUT");
		}
	}
}

int32_t AndroidTouch::HandleTouchEvent(struct android_app* app, AInputEvent* event)
{
	TouchEvent touchEvent;

	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
	{
		int32_t motionAction = AMotionEvent_getAction(event);
		int32_t motionCode = motionAction & AMOTION_EVENT_ACTION_MASK;
		int32_t pointerIndex =
			(motionAction & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
		int32_t pointerCount = AMotionEvent_getPointerCount(event);

		touchEvent.pointer_id	= AMotionEvent_getPointerId(event, pointerIndex);
		touchEvent.x			= AMotionEvent_getX(event, pointerIndex);
		touchEvent.y			= AMotionEvent_getY(event, pointerIndex);

		switch (motionCode)
		{
			case AMOTION_EVENT_ACTION_DOWN:
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
			{
				if (mListener)
				{
					mListener->TouchDown(touchEvent);
				}
				break;
			}
			case AMOTION_EVENT_ACTION_UP:
			case AMOTION_EVENT_ACTION_POINTER_UP:
			{
				if (mListener)
				{
					mListener->TouchUp(touchEvent);
				}
				break;
			}
			case AMOTION_EVENT_ACTION_CANCEL:
			{
				if (mListener)
				{
					mListener->TouchCancel(touchEvent);
				}
				break;
			}
			case AMOTION_EVENT_ACTION_MOVE:
			{
				for (int32_t i = 0; i < pointerCount; i++)
				{
					touchEvent.pointer_id	= AMotionEvent_getPointerId(event, i);
					touchEvent.x			= AMotionEvent_getX(event, i);
					touchEvent.y			= AMotionEvent_getY(event, i);

					if (mListener)
					{
						mListener->TouchMove(touchEvent);
					}
				}
				break;
			}
		}

		return 1;
	}

	return 0;
}

} // namespace Crown

