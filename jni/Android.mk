LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := GPU_FFT
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES := GPU_FFT.cpp \
                   GPUUtil.cpp \
                   ShaderSource.cpp
                   
LOCAL_LDLIBS    := -landroid -llog -lGLESv3 -lEGL

include $(BUILD_SHARED_LIBRARY)
