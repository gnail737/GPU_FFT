#include <jni.h>
#include "GPU_FFT.h"

#define DATA_SIZE 16
typedef unsigned int UINT;

//input arrays has RGBA four-tuple per element, R is real component G is imaginary component, B,A are ignored
static GLfloat inputArray[DATA_SIZE*4], testOutput[DATA_SIZE*4];
static GLuint VAO, inputTexture, texWidth = DATA_SIZE, texHeight = 1;
static Slab offScreenSlab;
static Programs myProgram;


//given a M (M<32) bits number(0 - 2^M) revert bits in place
void reverseInPlace(UINT * input, UINT m)
{
	UINT out = 0, scrape = 0, k;
	for (k=m; k>=1; k--) {
		//right shift till see the last bit
		scrape =(*input)>>(k-1) & 0x00000001;
		//out is assembled bit by bit from last to first
	    out += ((scrape == 0) ?  0 : (1<<(m-k)));
	}

	*input = out;
}

void init() {
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	compileAllShaderPrograms(&myProgram);
	//initialize input textures
	for (int i=0; i<DATA_SIZE; i++) {
		UINT t = i;
		reverseInPlace(&t, 4);
		inputArray[t*4] = 1.0f; //sin(2.0f*PI*(float)i/(float)16.0);  //sin(2.0f*PI*(float)t/(float)DATA_SIZE);
		inputArray[t*4 + 3] = inputArray[t*4 + 2] = inputArray[t*4 + 1] = 0.0f;
	}

	inputTexture = createTextureWFloats(texWidth, texHeight, 4, inputArray);

	offScreenSlab = CreateSlab(texWidth, texHeight, 4);

	CreateOffScreenVao(&VAO);

	initSlabSurface();
}
//now we just pass through data to rendered textured, in next version will do scramble stage
void initSlabSurface() {
	ALOGV("In initSlabSurface() .. ");
	glUseProgram(myProgram.ScrambleStage);
	glBindFramebuffer(GL_FRAMEBUFFER, offScreenSlab.Ping.FboHandle);
	glViewport(0, 0, texWidth, texHeight);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(VAO);
	GLint sampler = glGetUniformLocation(myProgram.DecimationInTime, "floatArray");
	glUniform1i(sampler, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTexture);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glReadPixels(0, 0, texWidth, texHeight, GL_RGBA, GL_FLOAT, testOutput);
	for (int i=0; i<DATA_SIZE; i++) {
		ALOGV(" %f - %f - %f - %f \n\n", testOutput[4*i], testOutput[4*i+1], testOutput[4*i+2], testOutput[4*i+3]);
	}

}

void resize(int w, int h) {
	ALOGV("In resize() : width = %d height = %d", w, h);
}

void drawLoop() {
	//initSlabSurface();
	glUseProgram(myProgram.DecimationInTime);


	GLint sampler = glGetUniformLocation(myProgram.DecimationInTime, "floatArray");
	glUniform1i(sampler, 1);
	glActiveTexture(GL_TEXTURE1);

	//start decimation in time loop
	for (int i=0; i<4; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, offScreenSlab.Pong.FboHandle);
		glViewport(0, 0, texWidth, texHeight);
		glClear(GL_COLOR_BUFFER_BIT);
		glBindVertexArray(VAO);

		int butterfly = 0x00000001<<(i);

	    GLint butterflyIndex = glGetUniformLocation(myProgram.DecimationInTime, "butterflyIndex");
	    glUniform1i(butterflyIndex, butterfly);

		glBindTexture(GL_TEXTURE_2D, offScreenSlab.Ping.TextureHandle);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//		glReadPixels(0, 0, texWidth, texHeight, GL_RGBA, GL_FLOAT, testOutput);
//		ALOGV(" input stage at %d \n", i);
//		for (int k=0; k<DATA_SIZE; k++) {
//
//			ALOGV(" %f  -  %f  -  %f  -  %f \n\n", testOutput[4*k], testOutput[4*k+1], testOutput[4*k+2], testOutput[4*k+3]);
//		}
//        ALOGV("\n------------------------------------------------------------------\n");
        switchSurfaces(&offScreenSlab);
	}

	glReadPixels(0, 0, texWidth, texHeight, GL_RGBA, GL_FLOAT, testOutput);
	for (int i=0; i<DATA_SIZE; i++) {
		ALOGV(" %f  -  %f  -  %f  -  %f \n\n", testOutput[4*i], testOutput[4*i+1], testOutput[4*i+2], testOutput[4*i+3]);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


extern "C" {
    JNIEXPORT void JNICALL Java_com_gnail737_gpu_1fft_GLES3JNILib_init(JNIEnv* env, jobject obj, jobject jAssetManager);
    JNIEXPORT void JNICALL Java_com_gnail737_gpu_1fft_GLES3JNILib_resize(JNIEnv* env, jobject obj, jint width, jint height);
    JNIEXPORT void JNICALL Java_com_gnail737_gpu_1fft_GLES3JNILib_step(JNIEnv* env, jobject obj);
};

JNIEXPORT void JNICALL
Java_com_gnail737_gpu_1fft_GLES3JNILib_init(JNIEnv* env, jobject obj, jobject jAssetManager) {
	AAssetManager* assets = AAssetManager_fromJava(env, jAssetManager);
	init();
}

JNIEXPORT void JNICALL
Java_com_gnail737_gpu_1fft_GLES3JNILib_resize(JNIEnv* env, jobject obj, jint width, jint height) {
	resize(width, height);
}

JNIEXPORT void JNICALL
Java_com_gnail737_gpu_1fft_GLES3JNILib_step(JNIEnv* env, jobject obj) {
	drawLoop();
}
