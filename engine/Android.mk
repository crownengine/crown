
LOCAL_PATH := $(call my-dir)

###############################################################################
# libluajit-5.1
###############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := luajit-5.1
LOCAL_SRC_FILES := libluajit-5.1.so
include $(PREBUILT_SHARED_LIBRARY)

###############################################################################
# libogg & libvorbis
###############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := ogg
LOCAL_SRC_FILES := libogg.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := vorbis
LOCAL_SRC_FILES := libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

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
\
	core/filesystem/File.cpp\
	core/filesystem/DiskFile.cpp\
	core/filesystem/DiskFilesystem.cpp\
\
	core/json/JSON.cpp\
	core/json/JSONParser.cpp\
\
	core/math/Color4.cpp\
	core/math/Matrix3x3.cpp\
	core/math/Matrix4x4.cpp\
	core/math/Plane.cpp\
	core/math/Quaternion.cpp\
	core/math/Vector2.cpp\
	core/math/Vector3.cpp\
	core/math/Vector4.cpp\
\
	core/mem/HeapAllocator.cpp\
	core/mem/LinearAllocator.cpp\
	core/mem/ProxyAllocator.cpp\
	core/mem/StackAllocator.cpp\
	core/mem/PoolAllocator.cpp\
	core/mem/Memory.cpp\
\
	core/settings/FloatSetting.cpp\
	core/settings/IntSetting.cpp\
	core/settings/StringSetting.cpp\
\
	core/Args.cpp\
	core/Log.cpp\
\
	os/android/AndroidOS.cpp\
	os/android/AndroidDevice.cpp\
	os/android/OsWindow.cpp\
	os/android/ApkFile.cpp\
	os/android/ApkFilesystem.cpp\
	os/posix/OsFile.cpp\
\
	renderers/gl/GLRenderer.cpp\
	renderers/gl/egl/GLContext.cpp\
\
	resource/FileBundle.cpp\
	resource/ResourceLoader.cpp\
	resource/ResourceManager.cpp\
	resource/ResourceRegistry.cpp\
\
	rpc/RPCServer.cpp\
	rpc/RPCHandler.cpp\
\
	lua/LuaStack.cpp\
	lua/LuaEnvironment.cpp\
	lua/LuaAccelerometer.cpp\
	lua/LuaDevice.cpp\
	lua/LuaKeyboard.cpp\
	lua/LuaMatrix4x4.cpp\
	lua/LuaMath.cpp\
	lua/LuaMouse.cpp\
	lua/LuaQuaternion.cpp\
	lua/LuaTouch.cpp\
	lua/LuaVector2.cpp\
	lua/LuaVector3.cpp\
	lua/LuaWindow.cpp\
	lua/LuaIntSetting.cpp\
	lua/LuaFloatSetting.cpp\
	lua/LuaStringSetting.cpp\
	lua/LuaResourcePackage.cpp\
	lua/LuaUnit.cpp\
	lua/LuaCamera.cpp\
	lua/LuaWorld.cpp\
	lua/LuaMesh.cpp\
	lua/LuaSprite.cpp\
\
	audio/sles/SLESRenderer.cpp\
\
	Camera.cpp\
	Device.cpp\
	Mesh.cpp\
	RenderWorld.cpp\
	SceneGraph.cpp\
	Sprite.cpp\
	Unit.cpp\
	World.cpp\
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
	$(LOCAL_PATH)/rpc\
	$(LOCAL_PATH)/input\
	$(LOCAL_PATH)/lua\
	$(LOCAL_PATH)/audio\
	$(LOCAL_PATH)/network\
	$(LOCAL_PATH)/os\
	$(LOCAL_PATH)/os/android\
	$(LOCAL_PATH)/os/posix\
	$(LOCAL_PATH)/renderers\
	$(LOCAL_PATH)/renderers/gl\
	$(LOCAL_PATH)/renderers/gl/egl\
	$(LOCAL_PATH)/third/ARMv7/luajit/include/luajit-2.0\
	$(LOCAL_PATH)/third/ARMv7/oggvorbis/include\
	
LOCAL_CPPFLAGS	:= -g -fexceptions -std=c++03 -ansi -pedantic -Wall -Wextra -Wno-long-long -Wno-variadic-macros
LOCAL_LDLIBS	:= -llog -landroid -lEGL -lGLESv2 -lz -lOpenSLES
LOCAL_SHARED_LIBRARIES := luajit-5.1
LOCAL_STATIC_LIBRARIES := android_native_app_glue vorbis ogg
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
