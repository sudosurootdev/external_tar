#-------------------------------------------------------------------------------
# FOTA Services MAIN
#-------------------------------------------------------------------------------
LOCAL_LGUA_PATH := $(call lgua-my-dir)

include $(CLEAR_VARS_FILE)

LOCAL_LGUA_MODULE := gzip

LOCAL_LGUA_SRC_FILES :=\
		bits.c \
		crypt.c \
		deflate.c \
		getopt.c \
		gzip.c \
		inflate.c \
		lzw.c \
		trees.c \
		unlzh.c \
		unlzw.c \
		unpack.c \
		unzip.c \
		util.c \
		zip.c

LOCAL_LGUA_C_INCLUDES := $(LOCAL_LGUA_PATH)/lib

include $(MAKE_STATIC_LIBS)
