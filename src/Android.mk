# We cannot use stlport on the simulator because it conficts with the host stl
# library. Android's port also relies on bionic which is not built for the
# simulator either.
ifneq ($(TARGET_SIMULATOR),true)
LOCAL_PATH := $(call my-dir)

#####################################################
# kalen.lee@lge.com create it based on libstlport.mk 
#####################################################
tar_src_files := \
	buffer.c\
	compare.c\
	create.c\
	delete.c\
	extract.c\
	incremen.c\
	list.c\
	mangle.c\
	misc.c\
	names.c\
	sparse.c\
	system.c\
	tar.c\
	transform.c\
	update.c\
	utf8.c\
	xheader.c

tar_cflags := -D_GNU_SOURCE -DHAVE_CONFIG_H 

##########################################
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	vendor/lge/apps/brd/tar \
        vendor/lge/apps/brd/tar/gnu

LOCAL_SRC_FILES := $(tar_src_files)

LOCAL_CFLAGS := $(tar_cflags)

LOCAL_SHARED_LIBRARIES += libgnu

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := tar

LOCAL_MODULE_PATH := $(TARGET_ROOT_OUT_SBIN)
LOCAL_UNSTRIPPED_PATH := $(TARGET_ROOT_OUT_SBIN_UNSTRIPPED)

include $(BUILD_EXECUTABLE)

##########################################
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
	bionic \
	vendor/lge/apps/brd/tar/gnu \
	$(LOCAL_C_INCLUDES)

#LOCAL_C_INCLUDES := \
#	external/tar/gnu

LOCAL_SRC_FILES := $(tar_src_files)

LOCAL_CFLAGS := $(tar_cflags)

LOCAL_PRELINK_MODULE := false

LOCAL_STATIC_LIBRARIES += libgnu libdl libc libm libstdc++
#LOCAL_STATIC_LIBRARIES += libc libdl libm libstdc++ libgnu

LOCAL_FORCE_STATIC_EXECUTABLE := true

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := tar_static

include $(BUILD_EXECUTABLE)

endif
