LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := mfdisk
LOCAL_SRC_FILES := mfdisk.c
LOCAL_CFLAGS	:= -std=gnu99
LOCAL_LDLIBS    := -llog

include $(BUILD_EXECUTABLE)
