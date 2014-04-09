#ifndef GPU_FFT_H
#define GPU_FFT_H 1

#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <stdlib.h>
#include <math.h>
#include <GLES3/gl3.h>

#define DEBUG 1
#define LOG_TAG "GPU_FFT"
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#if DEBUG
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#else
#define ALOGV(...)
#endif

#define PI 3.141592654f

typedef struct Surface_ {
    GLuint FboHandle;
    GLuint TextureHandle;
    int NumComponents;
} Surface;

typedef struct Slab_ {
    Surface Ping;
    Surface Pong;
} Slab;

typedef struct ProgramsRec {
	GLuint ScrambleStage;
	GLuint DecimationInTime;

	GLuint OffScreenRendering;
	GLuint PointSpriteVisualize;
	GLuint InitVelocity;
} Programs;


Surface CreateSurface(GLsizei width, GLsizei height, int numComponents);
GLuint createTextureWFloats(GLsizei width, GLsizei height, int numComponents, GLfloat *floatArray);
GLuint createTextureWInts(GLsizei width, GLsizei height, int numComponents, GLushort *intArray);
Slab CreateSlab(GLsizei width, GLsizei height, int numComponents);
void switchSurfaces(Slab *slab);
void CreateOffScreenVao(GLuint * vao);
void initSlabSurface();

bool checkGlError(const char* funcName);
void compileAllShaderPrograms(Programs *progObj);


#endif //#ifndef GPU_FFT_H
