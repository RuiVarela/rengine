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
#include <android/sensor.h>
#include <android_native_app_glue.h>

#include <string.h>
#include <memory.h>
#include <sys/time.h>

#include "imSquared.hpp"


/**
 * Our saved state data.
 */
struct saved_state
{

};

/**
 * Shared state for our app.
 */
struct engine
{
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    imSquared squared;

    struct saved_state state;
};



extern "C"
{
	typedef long long TickType;

	static TickType g_start_tick;
	static double g_seconds_per_tick;
	static engine g_engine;


	void* OS_Malloc(int size)
	{
		return malloc(size);
	}

	void OS_Free(void* pointer)
	{
		if(pointer)
			free(pointer);
	}

	int OS_ReadFileBinary(const char* filename, char** output)
	{
		int result = -1;

		 AAssetManager* assetManager = g_engine.app->activity->assetManager;
		 AAsset* asset = AAssetManager_open(assetManager, filename, AASSET_MODE_BUFFER);
		 if (asset)
		 {
			 off_t size = AAsset_getLength(asset);
			 //LOGI("asset[%s] has size: %d", filename, size);

			 const void* source = AAsset_getBuffer(asset);

			 (*output) = (char*)OS_Malloc(size);
			 if (*output)
			 {
				 memcpy(*output, source, size);
				 result = size;
			 }

			 AAsset_close(asset);
		 }

		 if (result <= 0)
		 {
			 LOGI("Unable to open asset %s", filename);
		 }

		return result;
	}



	static TickType getTick()
	{
		struct timeval time_val;
		gettimeofday(&time_val, 0);

		return ((TickType)time_val.tv_sec) * 1000000 + (TickType) time_val.tv_usec;

	}

	void OS_Initialize()
	{
		g_seconds_per_tick = (1.0 / 1000000.0);
		g_start_tick = getTick();
	}

	void OS_Finalize()
	{

	}

	double OS_ElapsedTime()
	{
		return (double) (getTick() - g_start_tick) * g_seconds_per_tick;
	}


	char* OS_GetFilePath(const char* filename)
	{
		__android_log_print(ANDROID_LOG_INFO, "native-activity", "OS_GetFilePath %s", filename);

		int size = 13;
		int filename_size = strlen(filename);

		char* output = (char*)OS_Malloc(size + filename_size + 1);

		if (output)
		{
			memcpy(output, "assets\\", size);
			memcpy(output + size, filename, filename_size);
			output[size + filename_size] = 0;
		}

		return output;
	}
}


/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine)
{
	OS_Initialize();


    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, NULL);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    // Initialize GL state.
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
    //glDisable(GL_DEPTH_TEST);



	imSquared::Configuration cfg;
	cfg.device = imSquared::Pc;
	cfg.screenWidth = w;
	cfg.screenHeight = h;
	cfg.columns = 10;
	cfg.rows = 13;
	cfg.level = "level 1";
	cfg.texture_enabled = false;

	imSquared im_squared;
	engine->squared.initialize(cfg);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine)
{
    if (engine->display == NULL) { return; }


    // Just fill the screen with a color.
    //glClearColor(((float)engine->state.x)/engine->width, engine->state.angle, ((float)engine->state.y)/engine->height, 1);
    //glClear(GL_COLOR_BUFFER_BIT);


    engine->squared.update();
    engine->squared.render();
    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine)
{
    if (engine->display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(engine->display, engine->context);
        }

        if (engine->surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }

    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;

    OS_Finalize();
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        //engine->state.x = AMotionEvent_getX(event, 0);
        //engine->state.y = AMotionEvent_getY(event, 0);
        return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct engine* engine = (struct engine*)app->userData;

    switch (cmd)
    {
        case APP_CMD_SAVE_STATE:
        	LOGI("APP_CMD_SAVE_STATE");

            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));

            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;

        case APP_CMD_INIT_WINDOW:
        	LOGI("APP_CMD_INIT_WINDOW");

            // The window is being shown, get it ready.
            if (engine->app->window != NULL)
            {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;

        case APP_CMD_TERM_WINDOW:
        	LOGI("APP_CMD_TERM_WINDOW");

            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;

        case APP_CMD_GAINED_FOCUS:
        	LOGI("APP_CMD_GAINED_FOCUS");

            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL)
            {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue, engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue, engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;

        case APP_CMD_LOST_FOCUS:
        	LOGI("APP_CMD_LOST_FOCUS");

            // When our app loses focus, we stop monitoring the accelerometer. This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL)
            {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,  engine->accelerometerSensor);
            }

            engine_draw_frame(engine);
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state)
{
    engine& engine = g_engine;

    // Make sure glue isn't stripped.
    app_dummy();


    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager, ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager, state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL)
    {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.

    while (1)
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
        {
            // Process this event.
            if (source != NULL)
            {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER)
            {
                if (engine.accelerometerSensor != NULL)
                {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,  &event, 1) > 0)
                    {
                        //LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0)
            {
                engine_term_display(&engine);
                return;
            }
        }

        engine_draw_frame(&engine);
    }
}
//END_INCLUDE(all)
