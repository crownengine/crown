
LOCAL_PATH := $(call my-dir)

###############################################################################
# libluajit-5.1
###############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := luajit-5.1
LOCAL_SRC_FILES := libluajit-5.1.so
include $(PREBUILT_SHARED_LIBRARY)

###############################################################################
# nvidia physx
###############################################################################
include $(CLEAR_VARS)
LOCAL_MODULE := PhysX3
LOCAL_SRC_FILES := libPhysX3.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := SimulationController
LOCAL_SRC_FILES := libSimulationController.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LowLevel
LOCAL_SRC_FILES := libLowLevel.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LowLevelCloth
LOCAL_SRC_FILES := libLowLevelCloth.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := PxTask
LOCAL_SRC_FILES := libPxTask.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := PhysX3Extensions
LOCAL_SRC_FILES := libPhysX3Extensions.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := SceneQuery
LOCAL_SRC_FILES := libSceneQuery.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := PhysX3Common
LOCAL_SRC_FILES := libPhysX3Common.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := PhysX3CharacterKinematic
LOCAL_SRC_FILES := libPhysX3CharacterKinematic.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := PhysXProfileSDK
LOCAL_SRC_FILES := libPhysXProfileSDK.a
include $(PREBUILT_STATIC_LIBRARY)

PhysX_libraries :=\
	LowLevel\
	LowLevelCloth\
	PhysX3\
	PhysX3CharacterKinematic\
	PhysX3Common\
	PhysX3Cooking\
	PhysX3Extensions\
	PhysXProfileSDK\
	PxTask\
	SceneQuery\
	SimulationController\

###############################################################################
# libcrown
###############################################################################
include $(CLEAR_VARS)

LOCAL_MODULE := crown
LOCAL_SRC_FILES :=\
\
	audio/backend/SLESSoundWorld.cpp\
\
	core/compressors/ZipCompressor.cpp\
\
	core/filesystem/DiskFile.cpp\
	core/filesystem/DiskFilesystem.cpp\
	core/filesystem/File.cpp\
	core/filesystem/NetworkFile.cpp\
	core/filesystem/NetworkFilesystem.cpp\
\
	core/json/JSON.cpp\
	core/json/JSONParser.cpp\
\
	core/math/Color4.cpp\
\
	core/mem/LinearAllocator.cpp\
	core/mem/Memory.cpp\
	core/mem/PoolAllocator.cpp\
	core/mem/ProxyAllocator.cpp\
	core/mem/StackAllocator.cpp\
\
	core/settings/FloatSetting.cpp\
	core/settings/IntSetting.cpp\
	core/settings/StringSetting.cpp\
\
	core/strings/Path.cpp\
\
	os/android/Android.cpp\
	os/android/AndroidDevice.cpp\
	os/android/ApkFile.cpp\
	os/android/ApkFilesystem.cpp\
	os/android/OsWindow.cpp\
	os/posix/OsFile.cpp\
	os/posix/Posix.cpp\
\
	physics/Actor.cpp\
	physics/Controller.cpp\
	physics/PhysicsWorld.cpp\
	physics/Joint.cpp\
	physics/Raycast.cpp\
\
	renderers/backend/gl/egl/GLContext.cpp\
	renderers/backend/gl/GLRenderer.cpp\
\
	renderers/DebugLine.cpp\
	renderers/Material.cpp\
	renderers/Mesh.cpp\
	renderers/RenderWorld.cpp\
	renderers/Sprite.cpp\
	renderers/Gui.cpp\
\
	resource/FileBundle.cpp\
	resource/ResourceLoader.cpp\
	resource/ResourceManager.cpp\
	resource/ResourceRegistry.cpp\
\
lua/LuaAccelerometer.cpp\
	lua/LuaActor.cpp\
	lua/LuaCamera.cpp\
	lua/LuaColor4.cpp\
	lua/LuaController.cpp\
	lua/LuaDebugLine.cpp\
	lua/LuaDevice.cpp\
	lua/LuaEnvironment.cpp\
	lua/LuaFloatSetting.cpp\
	lua/LuaGui.cpp\
	lua/LuaIntSetting.cpp\
	lua/LuaKeyboard.cpp\
	lua/LuaMath.cpp\
	lua/LuaMatrix4x4.cpp\
	lua/LuaMatrix4x4Box.cpp\
	lua/LuaMesh.cpp\
	lua/LuaMouse.cpp\
	lua/LuaPhysicsWorld.cpp\
	lua/LuaQuaternion.cpp\
	lua/LuaQuaternionBox.cpp\
	lua/LuaRaycast.cpp\
	lua/LuaResourcePackage.cpp\
	lua/LuaSoundWorld.cpp\
	lua/LuaSprite.cpp\
	lua/LuaStringSetting.cpp\
	lua/LuaSystem.cpp\
	lua/LuaTouch.cpp\
	lua/LuaUnit.cpp\
	lua/LuaVector2.cpp\
	lua/LuaVector2Box.cpp\
	lua/LuaVector3.cpp\
	lua/LuaVector3Box.cpp\
	lua/LuaWindow.cpp\
	lua/LuaWorld.cpp\
\
	world/Camera.cpp\
	world/SceneGraph.cpp\
	world/SceneGraphManager.cpp\
	world/Unit.cpp\
	world/World.cpp\
	world/WorldManager.cpp\
\
	ConsoleServer.cpp\
	Device.cpp\

LOCAL_C_INCLUDES	:=\
	$(LOCAL_PATH)/\
	$(LOCAL_PATH)/audio\
	$(LOCAL_PATH)/core/compressors\
	$(LOCAL_PATH)/core/containers\
	$(LOCAL_PATH)/core/filesystem\
	$(LOCAL_PATH)/core/filesystem\
	$(LOCAL_PATH)/core/json\
	$(LOCAL_PATH)/core/math\
	$(LOCAL_PATH)/core/mem\
	$(LOCAL_PATH)/core/settings\
	$(LOCAL_PATH)/core/strings\
	$(LOCAL_PATH)/core\
	$(LOCAL_PATH)/gui\
	$(LOCAL_PATH)/input\
	$(LOCAL_PATH)/lua\
	$(LOCAL_PATH)/network\
	$(LOCAL_PATH)/os/android\
	$(LOCAL_PATH)/os/posix\
	$(LOCAL_PATH)/os\
	$(LOCAL_PATH)/physics\
	$(LOCAL_PATH)/renderers/backend/gl/egl\
	$(LOCAL_PATH)/renderers/backend/gl\
	$(LOCAL_PATH)/renderers/backend\
	$(LOCAL_PATH)/renderers\
	$(LOCAL_PATH)/resource\
	$(LOCAL_PATH)/world\
\
	$(LOCAL_PATH)/third/ARMv7/luajit/include/luajit-2.0\
\
	$(LOCAL_PATH)/third/ARMv7/physx/include\
	$(LOCAL_PATH)/third/ARMv7/physx/include/common\
	$(LOCAL_PATH)/third/ARMv7/physx/include/characterkinematic\
	$(LOCAL_PATH)/third/ARMv7/physx/include/cloth\
	$(LOCAL_PATH)/third/ARMv7/physx/include/common\
	$(LOCAL_PATH)/third/ARMv7/physx/include/cooking\
	$(LOCAL_PATH)/third/ARMv7/physx/include/extensions\
	$(LOCAL_PATH)/third/ARMv7/physx/include/foundation\
	$(LOCAL_PATH)/third/ARMv7/physx/include/geometry\
	$(LOCAL_PATH)/third/ARMv7/physx/include/particles\
	$(LOCAL_PATH)/third/ARMv7/physx/include/physxprofilesdk\
	$(LOCAL_PATH)/third/ARMv7/physx/include/physxvisualdebuggersdk\
	$(LOCAL_PATH)/third/ARMv7/physx/include/pvd\
	$(LOCAL_PATH)/third/ARMv7/physx/include/pxtask\
	$(LOCAL_PATH)/third/ARMv7/physx/include/RepX\
	$(LOCAL_PATH)/third/ARMv7/physx/include/RepXUpgrader\
	$(LOCAL_PATH)/third/ARMv7/physx/include/vehicle\
	
LOCAL_CPPFLAGS :=\
	-std=c++03\
	-ansi\
	-Wall\
	-Wextra\
	-Wno-long-long\
	-Wno-variadic-macros\
	-Wno-missing-braces\
	-Wno-unused-parameter\
	-Wno-unknown-pragmas\
	-Wno-format\
	-Wno-unused-but-set-variable\
	-fno-rtti\
	-fno-exceptions\

LOCAL_LDLIBS := -L$(LOCAL_PATH) -Wl,--start-group $(addprefix -l, $(PhysX_libraries)) -Wl,--end-group -llog -landroid -lEGL -lGLESv2 -lz -lOpenSLES
LOCAL_SHARED_LIBRARIES := luajit-5.1
LOCAL_STATIC_LIBRARIES := android_native_app_glue
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
