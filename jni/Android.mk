LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= libsavebmp

LOCAL_SRC_FILES:= \
	bmp_to_screen.c \
	myfb.c \
	readbmp.c \
	rgb_convert.c \
	savebmp.c \
	screenshot.c
	
LOCAL_STATIC_LIBRARIES := libcutils libc
include $(BUILD_SHARED_LIBRARY)

################################################

include $(CLEAR_VARS)
LOCAL_MODULE:= shot
LOCAL_SRC_FILES:=main_shot.c
LOCAL_STATIC_LIBRARIES := libsavebmp libcutils libc
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE:= tofb
LOCAL_SRC_FILES:=main_tofb.c
LOCAL_STATIC_LIBRARIES := libsavebmp libcutils libc
include $(BUILD_EXECUTABLE)
