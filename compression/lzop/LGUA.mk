#-------------------------------------------------------------------------------
# FOTA Services MAIN
#-------------------------------------------------------------------------------
LOCAL_LGUA_PATH := $(call lgua-my-dir)

include $(CLEAR_VARS_FILE)

LOCAL_LGUA_MODULE := lzop

LOCAL_LGUA_SRC_FILES :=\
		src/lzo_crc.c \
		src/lzo_init.c \
		src/lzo_ptr.c \
		src/lzo_str.c \
		src/lzo_util.c \
		src/lzo1.c \
		src/lzo1_99.c \
		src/lzo1a.c \
		src/lzo1a_99.c \
		src/lzo1b_1.c \
		src/lzo1b_2.c \
		src/lzo1b_3.c \
		src/lzo1b_4.c \
		src/lzo1b_5.c \
		src/lzo1b_6.c \
		src/lzo1b_7.c \
		src/lzo1b_8.c \
		src/lzo1b_9.c \
		src/lzo1b_99.c \
		src/lzo1b_9x.c \
		src/lzo1b_cc.c \
		src/lzo1b_d1.c \
		src/lzo1b_d2.c \
		src/lzo1b_rr.c \
		src/lzo1b_xx.c \
		src/lzo1c_1.c \
		src/lzo1c_2.c \
		src/lzo1c_3.c \
		src/lzo1c_4.c \
		src/lzo1c_5.c \
		src/lzo1c_6.c \
		src/lzo1c_7.c \
		src/lzo1c_8.c \
		src/lzo1c_9.c \
		src/lzo1c_99.c \
		src/lzo1c_9x.c \
		src/lzo1c_cc.c \
		src/lzo1c_d1.c \
		src/lzo1c_d2.c \
		src/lzo1c_rr.c \
		src/lzo1c_xx.c \
		src/lzo1f_1.c \
		src/lzo1f_9x.c \
		src/lzo1f_d1.c \
		src/lzo1f_d2.c \
		src/lzo1x_1.c \
		src/lzo1x_9x.c \
		src/lzo1x_d1.c \
		src/lzo1x_d2.c \
		src/lzo1x_d3.c \
		src/lzo1x_o.c \
		src/lzo1x_1k.c \
		src/lzo1x_1l.c \
		src/lzo1x_1o.c \
		src/lzo1y_1.c \
		src/lzo1y_9x.c \
		src/lzo1y_d1.c \
		src/lzo1y_d2.c \
		src/lzo1y_d3.c \
		src/lzo1y_o.c \
		src/lzo1z_9x.c \
		src/lzo1z_d1.c \
		src/lzo1z_d2.c \
		src/lzo1z_d3.c \
		src/lzo2a_9x.c \
		src/lzo2a_d1.c \
		src/lzo2a_d2.c \
		src/lzop.c

LOCAL_LGUA_SRC_FILES:= $(common_SRC_FILES)
LOCAL_LGUA_C_INCLUDES:= $(LOCAL_LGUA_PATH)/include $(LOCAL_LGUA_PATH)/include/lzo

include $(MAKE_STATIC_LIBS)