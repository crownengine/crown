
LOCAL_PATH := $(call my-dir)

###############################################################################
# libluajit-5.1
###############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := luajit-5.1
LOCAL_SRC_FILES := libluajit-5.1.so
include $(PREBUILT_SHARED_LIBRARY)

###############################################################################
# libcrown
###############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE    := crown
LOCAL_SRC_FILES :=\
	core/bv/Circle.cpp\
	core/bv/Frustum.cpp\
	core/bv/Rect.cpp\
\
	core/compressors/ZipCompressor.cpp\
	core/containers/Generic.cpp\
\
	core/filesystem/File.cpp\
	core/filesystem/DiskFile.cpp\
	core/filesystem/DiskFilesystem.cpp\
\
	core/json/JSONParser.cpp\
\
	core/math/Color4.cpp\
	core/math/Mat3.cpp\
	core/math/Mat4.cpp\
	core/math/Plane.cpp\
	core/math/Quat.cpp\
	core/math/Vec2.cpp\
	core/math/Vec3.cpp\
	core/math/Vec4.cpp\
\
	core/mem/HeapAllocator.cpp\
	core/mem/LinearAllocator.cpp\
	core/mem/ProxyAllocator.cpp\
	core/mem/StackAllocator.cpp\
	core/mem/Memory.cpp\
\
	core/settings/FloatSetting.cpp\
	core/settings/IntSetting.cpp\
	core/settings/StringSetting.cpp\
\
	core/Args.cpp\
	core/Log.cpp\
\
	network/BitMessage.cpp\
\
	os/android/AndroidOS.cpp\
	os/android/AndroidDevice.cpp\
	os/android/OsWindow.cpp\
	os/android/ApkFile.cpp\
	os/android/ApkFilesystem.cpp\
	os/posix/OsFile.cpp\
	os/posix/TCPSocket.cpp\
	os/posix/UDPSocket.cpp\
\
	renderers/gl/GLRenderer.cpp\
	renderers/gl/egl/GLContext.cpp\
	renderers/DebugRenderer.cpp\
\
	resource/FileBundle.cpp\
	resource/ResourceLoader.cpp\
	resource/ResourceManager.cpp\
	resource/ResourceRegistry.cpp\
\
	lua/LuaStack.cpp\
	lua/LuaEnvironment.cpp\
	lua/LuaAccelerometer.cpp\
	lua/LuaDevice.cpp\
	lua/LuaKeyboard.cpp\
	lua/LuaMat4.cpp\
	lua/LuaMath.cpp\
	lua/LuaMouse.cpp\
	lua/LuaQuat.cpp\
	lua/LuaTouch.cpp\
	lua/LuaVec2.cpp\
	lua/LuaVec3.cpp\
	lua/LuaWindow.cpp\
	lua/LuaIntSetting.cpp\
	lua/LuaFloatSetting.cpp\
	lua/LuaStringSetting.cpp\
	lua/LuaResourcePackage.cpp\
\
	Camera.cpp\
	Device.cpp\
	ConsoleServer.cpp\
\

LOCAL_C_INCLUDES	:=\
	$(LOCAL_PATH)/\
	$(LOCAL_PATH)/core\
	$(LOCAL_PATH)/core/bv\
	$(LOCAL_PATH)/core/compressors\
	$(LOCAL_PATH)/core/containers\
	$(LOCAL_PATH)/core/filesystem\
	$(LOCAL_PATH)/core/math\
	$(LOCAL_PATH)/core/mem\
	$(LOCAL_PATH)/core/filesystem\
	$(LOCAL_PATH)/core/json\
	$(LOCAL_PATH)/core/settings\
	$(LOCAL_PATH)/core/strings\
	$(LOCAL_PATH)/resource\
	$(LOCAL_PATH)/input\
	$(LOCAL_PATH)/lua\
	$(LOCAL_PATH)/network\
	$(LOCAL_PATH)/os\
	$(LOCAL_PATH)/os/android\
	$(LOCAL_PATH)/os/posix\
	$(LOCAL_PATH)/renderers\
	$(LOCAL_PATH)/renderers/gl\
	$(LOCAL_PATH)/renderers/gl/egl\
	$(LOCAL_PATH)/luajit/include/luajit-2.0\

LOCAL_CPPFLAGS	:= -g -fexceptions -std=c++03 -ansi -pedantic -Wall -Wextra -Wno-long-long -Wno-variadic-macros
LOCAL_LDLIBS	:= -llog -landroid -lEGL -lGLESv2 -lz 
LOCAL_SHARED_LIBRARIES := luajit-5.1
LOCAL_STATIC_LIBRARIES := android_native_app_glue
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)

