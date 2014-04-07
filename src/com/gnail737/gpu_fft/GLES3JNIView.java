package com.gnail737.gpu_fft;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class GLES3JNIView extends GLSurfaceView {

    private static final String TAG = "GLES3JNI";
    private static final boolean DEBUG = true;
    private static Context mContext;
    public GLES3JNIView(Context context) {
        super(context);
        mContext = context;
        // Pick an EGLConfig with RGB8 color, 16-bit depth, no stencil,
        // supporting OpenGL ES 2.0 or later backwards-compatible versions.
        Log.i(TAG, "Before Choosing EGL Context");
        setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        setEGLContextClientVersion(3);
        setRenderer(new Renderer());
    }

    private static class Renderer implements GLSurfaceView.Renderer {
        public void onDrawFrame(GL10 gl) {
            GLES3JNILib.step();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            GLES3JNILib.resize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        	AssetManager aMgr = (mContext).getAssets();
            GLES3JNILib.init(aMgr);
        }
    }

}
