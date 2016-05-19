#pragma once

#ifndef RENGINE_PLATFORM
	#ifdef GLEW_STATIC
		#define RENGINE_PLATFORM
	#endif 
#endif 

#ifdef RENGINE_PLATFORM
	#include <GL/glew.h>

	#define DPrintf(...) printf(__VA_ARGS__)
	#define EPrintf(...) printf(__VA_ARGS__)
#endif

#ifdef __ANDROID__
	#include <EGL/egl.h>
	#include <GLES/gl.h>

	#include <android/log.h>


	#define DPrintf(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
	#define EPrintf(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

	#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
	#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#endif


#ifdef __IOS__
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
    #include <OpenGLES/ES2/gl.h>
    #include <OpenGLES/ES2/glext.h>

	#define DPrintf(...) printf(__VA_ARGS__)
	#define EPrintf(...) printf(__VA_ARGS__)
#endif 


#define FILENAME_MAX_SIZE 256

extern "C"
{
	void OS_Initialize();
	void OS_Finalize();
	
	void* OS_Malloc(int size);
	void OS_Free(void* pointer);

	char* OS_GetFilePath(const char* filename);
	int OS_ReadFileBinary(const char* filename, char** output);

	double OS_ElapsedTime();

}