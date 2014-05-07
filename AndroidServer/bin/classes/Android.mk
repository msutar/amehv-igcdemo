LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog
LOCAL_MODULE := connection-jni
LOCAL_SRC_FILES := connection-jni.cpp

include $(BUILD_SHARED_LIBRARY)
