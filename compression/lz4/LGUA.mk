#-------------------------------------------------------------------------------
# FOTA Services MAIN
#-------------------------------------------------------------------------------
LOCAL_LGUA_PATH := $(call lgua-my-dir)

include $(CLEAR_VARS_FILE)

LOCAL_LGUA_MODULE := lz4

LOCAL_LGUA_SRC_FILES :=\
		lz4.c \
		lz4hc.c \
		bench.c \
		lz4demo.c

include $(MAKE_STATIC_LIBS)
