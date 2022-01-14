
APP_STL := stlport_static
APP_CPPFLAGS += -fno-rtti -w #-Wno-write-strings -Wno-conversion-null -Wno-pointer-arith -Wno-unused-local-typedefs

APP_ABI := arm64-v8a armeabi-v7a
#APP_ABI := x86

#APP_PLATFORM := android-9

# NdkMediaCodec
APP_PLATFORM := android-21