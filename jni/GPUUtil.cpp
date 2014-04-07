#include "GPU_FFT.h"
#define PositionSlot 0

bool checkGlError(const char* funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        ALOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

GLuint createTextureWFloats(GLsizei width, GLsizei height, int numComponents, GLfloat *floatArray)
{
	ALOGV("In createTextureWFloats : width = %d height = %d", width, height);

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    switch (numComponents) {
    case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, floatArray); break;
    case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, floatArray); break;
    case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, floatArray); break;
    case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, floatArray); break;
    //case 5 is added for OffScreen RenderBuffer which only store color not Floating Point
    case 5: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, floatArray); break;
    default: ALOGE("Illegal slab format.");
    }

    return textureHandle;
}
Surface CreateSurface(GLsizei width, GLsizei height, int numComponents)
{
	ALOGV("In createSurface : width = %d height = %d", width, height);
    GLuint fboHandle;
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const int UseHalfFloats = 1;
    if (UseHalfFloats) {
        switch (numComponents) {
            case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, 0); break;
            case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_HALF_FLOAT, 0); break;
            case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_HALF_FLOAT, 0); break;
            case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, 0); break;
            //case 5 is added for OffScreen RenderBuffer which only store color not Floating Point
            case 5: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); break;
            default: ALOGE("Illegal slab format.");
        }
    } else {
        switch (numComponents) {
            case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, 0); break;
            case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0); break;
            case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0); break;
            case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0); break;
            //case 5 is added for OffScreen RenderBuffer which only store color not Floating Point
            case 5: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); break;
            default: ALOGE("Illegal slab format.");
        }
    }

    GLuint colorbuffer;
    glGenRenderbuffers(1, &colorbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);
    checkGlError("After calling glFramebufferTexture2D to attach color buffer - ");

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
    	ALOGE("Unable to create FBO. error code = %d", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    Surface surface = { fboHandle, textureHandle, numComponents };

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return surface;
}

Slab CreateSlab(GLsizei width, GLsizei height, int numComponents) {
	Surface front = CreateSurface(width, height, numComponents);
	Surface back = CreateSurface(width, height, numComponents);
	Slab mySlab = {front, back};

	return mySlab;
}

void switchSurfaces(Slab *slab) {
	Surface temp = slab->Ping;
	slab->Ping = slab->Pong;
	slab->Pong = temp;
}

void CreateOffScreenVao(GLuint * vao)
{
   #define T 1.0f
   float positions[] = { -T, -T, 0, 0, T, -T, 1.0, 0, -T, T, 0, 1.0, T, T, 1.0, 1.0};
   #undef T
   GLuint vbo[1];
   glGenVertexArrays(1, vao);
   glBindVertexArray(*vao);
   // Create the VBO:
   GLsizeiptr size = sizeof(positions);
   glGenBuffers(1, vbo);
   glBindBuffer(GL_ARRAY_BUFFER, *vbo);
   glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
   // Set up the vertex layout:
   GLsizeiptr stride = 4 * sizeof(positions[0]);

   glEnableVertexAttribArray(PositionSlot);
   glVertexAttribPointer(PositionSlot, 2, GL_FLOAT, GL_FALSE, stride, 0);
   glBindVertexArray(0);
}
