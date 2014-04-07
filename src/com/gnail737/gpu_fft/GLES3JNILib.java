package com.gnail737.gpu_fft;

import android.content.res.AssetManager;

//Wrapper for native library

public class GLES3JNILib {

  static {
      System.loadLibrary("GPU_FFT");
  }

  public static native void init(AssetManager aMgr);
  public static native void resize(int width, int height);
  public static native void step();
}
