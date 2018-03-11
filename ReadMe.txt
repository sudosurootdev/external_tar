How to build tar source against Android platform?

1. perform "configure" as below.
export ANDROID_ROOT=/home3/kalen.lee/p940_v093_0916/p940_v09e_0916/android
export PATH=$ANDROID_ROOT/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/bin:$PATH
export NDK_PATH=$ANDROID_ROOT/prebuilt/ndk/android-ndk-r4/platforms/android-8/arch-arm
export TOOL_CHAIN_LIB_PATH=$ANDROID_ROOT/prebuilt/linux-x86/toolchain/arm-eabi-4.4.3/lib/gcc/arm-eabi/4.4.3
export PRODUCT_PATH=$ANDROID_ROOT/out/target/product/p940

NDK_PATH has to have proper directory dependent on platform version and target.
Please refer to LOCAL_NDK_VERSION,TARGET_ARCH,LOCAL_SDK_VERSION

PATH,TOOL_CHAIN_LIB_PATH has to have correct tool chain version. 

./configure  --prefix="/home3/kalen.lee" --with-rmt=/usr/sbin/rmt
 ./configure --host=arm-eabi CC=arm-eabi-gcc CPPFLAGS="-D__ARM_ARCH_5__ -include$ANDROID_ROOT/system/core/include/arch/linux-arm/AndroidConfig.h  -I$ANDROID_ROOT/system/core/include -I$ANDROID_ROOT/hardware/libhardware/include -I$ANDROID_ROOT/bionic/libc/include -I$ANDROID_ROOT/bionic/libc/kernel/common -I$ANDROID_ROOT/bionic/libc/kernel/arch-arm -I$ANDROID_ROOT/bionic/libc/kernel/arch-arm/asm -I$NDK_PATH/usr/include" CFLAGS="-nostdlib -march=armv5te -mtune=xscale -msoft-float -mthumb-interwork -fpic -fno-exceptions -ffunction-sections -funwind-tables -fstack-protector -fmessage-length=0 -O2 -finline-functions -finline-limit=300 -fno-inline-functions-called-once -fgcse-after-reload -frerun-cse-after-loop -frename-registers -fomit-frame-pointer -fstrict-aliasing -funswitch-loops"  LDFLAGS="-Bdynamic -Wl,-T,$ANDROID_ROOT/build/core/armelf.x -Wl,-dynamic-linker,linker -Wl,--gc-sections -Wl,-z,nocopyreloc -Wl,--no-undefined -Wl,-rpath-link=$PRODUCT_PATH/obj/lib -L$PRODUCT_PATH/obj/lib -nostdlib $TOOL_CHAIN_LIB_PATH/libgcc.a $NDK_PATH/usr/lib/libc.a $NDK_PATH/usr/lib/libm.a -L$NDK_PATH/usr/lib -lstdc++ $NDK_PATH/usr/lib/crtend_android.o "

*This is an example. if you fail to configure it, please correct it by adding some library paths or include paths.

2. build it with "make"
 correct errors at compile time

3. Make tar directory under android/external. Add gnu and src directories into it. 
   Add files in lib directory into tar/src.
   Add Android.mk,libgnu.mk from previous verson into each directory.
   Add modification on codes by searching kalen.lee@lge.com 

4. build it by "make gnulib", "make tar"
  correct errors at compile time

Unfortunately this process is done by manual rather than automatic.
 
