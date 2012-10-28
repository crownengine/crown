#include "TestApp.h"
#include "ArenaScene.h"

using namespace Crown;

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include "Crown.h"
using namespace Crown;

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

static void foobar()
{
	ArenaScene* arena = new ArenaScene(800, 480);
	GetDevice()->GetSceneManager()->SelectNextScene(arena);
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
			GetDevice()->Init(480, 800, 32, true);
			foobar();
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
			GetDevice()->StopRunning();
			GetDevice()->Shutdown();
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            GetDevice()->StartRunning();
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
			GetDevice()->StopRunning();
            // Also stop animating.
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {

    // Make sure glue isn't stripped.
    app_dummy();

	LOGI("Entrato in android_main!!!");
	GetDevice()->_InitAndroidApp(state);

    state->userData = NULL;
    state->onAppCmd = NULL; // Manual cmd handling
    state->onInputEvent = NULL; // Manual input handling

    // loop waiting for stuff to do.

    while (1) {

        int ident;
        int events;
        struct android_poll_source* source;
		struct android_app* app = state;

{ //////////////////////////////////////
        if ((ident=ALooper_pollOnce(0, NULL, &events,
                (void**)&source)) >= 0) {
					if (source != NULL)
					{
					if (source->id == LOOPER_ID_MAIN)
					{
						//LOGI("Entrato in LOOPER_ID_MAIN");
						int8_t cmd = android_app_read_cmd(app);
						android_app_pre_exec_cmd(app, cmd);
						engine_handle_cmd(app, cmd);
						android_app_post_exec_cmd(app, cmd);
						//LOGI("Uscito da LOOPER_ID_MAIN");
					}
					}
					//LOGI("Uscito da source != NULL");
		}

} ///////////////////////////////////

        // Check if we are exiting.
        if (state->destroyRequested != 0) {
            exit(0);
        }

        // Drawing is throttled to the screen update rate, so there
        // is no need to do timing here.
		if (GetDevice()->IsInit())
		{
			if (GetDevice()->IsRunning())
			{
				GetDevice()->GetInputManager()->GetTouch()->EventLoop();
				GetDevice()->GetRenderer()->_BeginFrame();
				GetDevice()->GetSceneManager()->UpdateScene(0.02);
				GetDevice()->GetSceneManager()->RenderScene();
				GetDevice()->GetRenderer()->_EndFrame();
				GetDevice()->GetMainWindow()->Update();
			}
		}
    }

	Log::D("Crown: bye bye.");
}
//END_INCLUDE(all)



