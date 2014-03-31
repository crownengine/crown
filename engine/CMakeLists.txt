cmake_minimum_required(VERSION 2.8)

# detect architecture
if (CROWN_ARCH MATCHES "x86")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/engine/third/x86)
endif (CROWN_ARCH MATCHES "x86")

if (CROWN_ARCH MATCHES "x86_64")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/engine/third/x86_64)
endif (CROWN_ARCH MATCHES "x86_64")

if (CROWN_ARCH MATCHES "win32")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/engine/third/win32)
endif (CROWN_ARCH MATCHES "win32")

if (CROWN_ARCH MATCHES "win64")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/engine/third/win64)
endif (CROWN_ARCH MATCHES "win64")

if (CROWN_ARCH MATCHES "ARMv7")
	set (CROWN_THIRD ${CMAKE_SOURCE_DIR}/engine/third/ARMv7)
endif (CROWN_ARCH MATCHES "ARMv7")

# architecture dependent includes
set (CROWN_THIRD_INCLUDES
	${CROWN_THIRD}/luajit/include/luajit-2.0
	${CROWN_THIRD}/glew-1.9.0/include
	${CROWN_THIRD}/zlib/include
	${CROWN_THIRD}/oggvorbis/include
	${CROWN_THIRD}/openAL/include
	${CROWN_THIRD}/physx/include
	${CROWN_THIRD}/physx/include/common
	${CROWN_THIRD}/physx/include/characterkinematic
	${CROWN_THIRD}/physx/include/cloth
	${CROWN_THIRD}/physx/include/common
	${CROWN_THIRD}/physx/include/cooking
	${CROWN_THIRD}/physx/include/extensions
	${CROWN_THIRD}/physx/include/foundation
	${CROWN_THIRD}/physx/include/geometry
	${CROWN_THIRD}/physx/include/particles
	${CROWN_THIRD}/physx/include/physxprofilesdk
	${CROWN_THIRD}/physx/include/physxvisualdebuggersdk
	${CROWN_THIRD}/physx/include/pvd
	${CROWN_THIRD}/physx/include/pxtask
	${CROWN_THIRD}/physx/include/RepX
	${CROWN_THIRD}/physx/include/RepXUpgrader
	${CROWN_THIRD}/physx/include/vehicle
)

# architecture dependent libraries
set (CROWN_THIRD_LIBS
	${CROWN_THIRD}/luajit/lib
	${CROWN_THIRD}/glew-1.9.0/lib
	${CROWN_THIRD}/zlib/lib
	${CROWN_THIRD}/oggvorbis/lib
	${CROWN_THIRD}/openAL/lib
	${CROWN_THIRD}/physx/lib
)

add_subdirectory(${CROWN_THIRD})

# platform independent includes
set (CROWN_INCLUDES
	${CMAKE_SOURCE_DIR}/engine
	${CMAKE_SOURCE_DIR}/engine/core
	${CMAKE_SOURCE_DIR}/engine/core/bv
	${CMAKE_SOURCE_DIR}/engine/core/containers
	${CMAKE_SOURCE_DIR}/engine/core/math
	${CMAKE_SOURCE_DIR}/engine/core/mem
	${CMAKE_SOURCE_DIR}/engine/core/compressors
	${CMAKE_SOURCE_DIR}/engine/core/filesystem
	${CMAKE_SOURCE_DIR}/engine/core/json
	${CMAKE_SOURCE_DIR}/engine/core/strings
	${CMAKE_SOURCE_DIR}/engine/core/settings
	${CMAKE_SOURCE_DIR}/engine/os
	${CMAKE_SOURCE_DIR}/engine/input
	${CMAKE_SOURCE_DIR}/engine/renderers
	${CMAKE_SOURCE_DIR}/engine/renderers/backend
	${CMAKE_SOURCE_DIR}/engine/resource
	${CMAKE_SOURCE_DIR}/engine/rpc
	${CMAKE_SOURCE_DIR}/engine/network
	${CMAKE_SOURCE_DIR}/engine/lua
	${CMAKE_SOURCE_DIR}/engine/audio
	${CMAKE_SOURCE_DIR}/engine/compilers
	${CMAKE_SOURCE_DIR}/engine/physics
	${CMAKE_SOURCE_DIR}/engine/world
	${CMAKE_SOURCE_DIR}/engine/gui
)

set (SRC
	Device.cpp
	ConsoleServer.cpp
)

set (HEADERS
	Config.h
	Crown.h
	Device.h
	ConsoleServer.h
)

set (CORE_SRC
	core/Args.cpp
	core/Log.cpp
)

set (CORE_HEADERS
	core/Args.h
	core/Assert.h
	core/Log.h
	core/Macros.h
	core/Profiler.h
	core/Types.h
)

set (CONTAINERS_SRC
)

set (CONTAINERS_HEADERS
	core/containers/Array.h
	core/containers/ContainerTypes.h
	core/containers/EventStream.h
	core/containers/Hash.h
	core/containers/IdArray.h
	core/containers/IdTable.h
	core/containers/Map.h
	core/containers/PriorityQueue.h
	core/containers/Queue.h
	core/containers/RBTree.h
	core/containers/Vector.h
)

set (MATH_SRC
	core/math/Color4.cpp
	core/math/Matrix3x3.cpp
	core/math/Matrix4x4.cpp
)

set (MATH_HEADERS
	core/math/AABB.h
	core/math/Color4.h
	core/math/Frustum.h
	core/math/Intersection.h
	core/math/MathTypes.h
	core/math/MathUtils.h
	core/math/Matrix3x3.h
	core/math/Matrix4x4.h
	core/math/Plane.h
	core/math/Quaternion.h
	core/math/Random.h
	core/math/Sphere.h
	core/math/Vector2.h
	core/math/Vector3.h
	core/math/Vector4.h
)

set (FILESYSTEM_SRC
	core/filesystem/DiskFile.cpp
	core/filesystem/DiskFilesystem.cpp
	core/filesystem/File.cpp
	core/filesystem/NetworkFile.cpp
	core/filesystem/NetworkFilesystem.cpp
)

set (FILESYSTEM_HEADERS
	core/filesystem/BinaryReader.h
	core/filesystem/BinaryWriter.h
	core/filesystem/DiskFile.h
	core/filesystem/DiskFilesystem.h
	core/filesystem/File.h
	core/filesystem/Filesystem.h
	core/filesystem/NetworkFile.h
	core/filesystem/NetworkFilesystem.h
	core/filesystem/NullFile.h
	core/filesystem/TextReader.h
	core/filesystem/TextWriter.h
)

set (JSON_SRC
	core/json/JSON.cpp
	core/json/JSONParser.cpp
)

set (JSON_HEADERS
	core/json/JSON.h
	core/json/JSONParser.h
)

set (STRINGS_SRC
)

set (STRINGS_HEADERS
	core/strings/Path.h
	core/strings/StringUtils.h
	core/strings/StringStream.h
	core/strings/DynamicString.h
)

set (MEM_SRC
	core/mem/Memory.cpp
	core/mem/LinearAllocator.cpp
	core/mem/StackAllocator.cpp
	core/mem/ProxyAllocator.cpp
	core/mem/PoolAllocator.cpp
)

set (MEM_HEADERS
	core/mem/Memory.h
	core/mem/Allocator.h
	core/mem/TempAllocator.h
	core/mem/LinearAllocator.h
	core/mem/StackAllocator.h
	core/mem/ProxyAllocator.h
	core/mem/PoolAllocator.h
)

set (COMPRESSORS_SRC
	core/compressors/ZipCompressor.cpp
)

set (COMPRESSORS_HEADERS
	core/compressors/Compressor.h
	core/compressors/ZipCompressor.h
)

set (SETTINGS_SRC
	core/settings/IntSetting.cpp
	core/settings/FloatSetting.cpp
	core/settings/StringSetting.cpp
)

set (SETTINGS_HEADERS
	core/settings/IntSetting.h
	core/settings/FloatSetting.h
	core/settings/StringSetting.h
)

set (INPUT_SRC
)

set (INPUT_HEADERS
	input/Keyboard.h
	input/KeyCode.h
	input/Mouse.h
	input/Touch.h
)

set (RENDERERS_BACKEND_SRC
)

set (RENDERERS_BACKEND_HEADERS
	renderers/backend/Renderer.h
	renderers/backend/RenderContext.h
	renderers/backend/PixelFormat.h
	renderers/backend/VertexFormat.h
)

set (RENDERERS_SRC
	renderers/RenderWorld.cpp
	renderers/Mesh.cpp
	renderers/Sprite.cpp
	renderers/Material.cpp
	renderers/DebugLine.cpp
)

set (RENDERERS_HEADERS
	renderers/RenderWorld.h
	renderers/RenderWorldTypes.h
	renderers/Mesh.h
	renderers/Sprite.h
	renderers/Material.h
	renderers/DebugLine.h
)

set (RESOURCE_SRC
	resource/ResourceLoader.cpp
	resource/ResourceManager.cpp
	resource/ResourceRegistry.cpp
	resource/UnitResource.cpp
	resource/TextureResource.cpp
	resource/SpriteResource.cpp
	resource/SoundResource.cpp
	resource/MaterialResource.cpp
	resource/PhysicsResource.cpp
	resource/PackageResource.cpp
	resource/MeshResource.cpp
	resource/LuaResource.cpp
	resource/GuiResource.cpp
	resource/FontResource.cpp
	resource/FileBundle.cpp
)

set (RESOURCE_HEADERS
	resource/Resource.h
	resource/ResourceLoader.h
	resource/ResourceManager.h
	resource/ResourceRegistry.h
	resource/Bundle.h
	resource/LuaResource.h
	resource/TextureResource.h
	resource/MeshResource.h
	resource/FontResource.h
	resource/SoundResource.h
	resource/MaterialResource.h
	resource/PackageResource.h
	resource/UnitResource.h
	resource/ResourcePackage.h
	resource/SpriteResource.h
	resource/PhysicsResource.h
	resource/GuiResource.h
	resource/FontResource.h
)

set (OS_SRC
)

set (OS_HEADERS
	os/OS.h
	os/OsEventQueue.h
	os/NetAddress.h
	os/OsTypes.h
	os/ScopedMutex.h
)

set (LUA_SRC
	lua/LuaAccelerometer.cpp
	lua/LuaActor.cpp
	lua/LuaCamera.cpp
	lua/LuaController.cpp
	lua/LuaDebugLine.cpp
	lua/LuaDevice.cpp
	lua/LuaEnvironment.cpp
	lua/LuaFloatSetting.cpp
	lua/LuaGui.cpp
	lua/LuaIntSetting.cpp
	lua/LuaKeyboard.cpp
	lua/LuaMath.cpp
	lua/LuaMatrix4x4.cpp
	lua/LuaMatrix4x4Box.cpp
	lua/LuaMesh.cpp
	lua/LuaMouse.cpp
	lua/LuaPhysicsWorld.cpp
	lua/LuaQuaternion.cpp
	lua/LuaQuaternionBox.cpp
	lua/LuaRaycast.cpp
	lua/LuaResourcePackage.cpp
	lua/LuaSoundWorld.cpp
	lua/LuaSprite.cpp
	lua/LuaStringSetting.cpp
	lua/LuaSystem.cpp
	lua/LuaTouch.cpp
	lua/LuaUnit.cpp
	lua/LuaVector2.cpp
	lua/LuaVector3.cpp
	lua/LuaVector3Box.cpp
	lua/LuaWindow.cpp
	lua/LuaWorld.cpp
)

set (LUA_HEADERS
	lua/LuaEnvironment.h
	lua/LuaStack.h
	lua/LuaSystem.h
)

set (AUDIO_SRC)

set (AUDIO_HEADERS
	audio/SoundWorld.h
	audio/OggDecoder.h
)

set (PHYSICS_SRC
	physics/Actor.cpp
	physics/Controller.cpp
	physics/PhysicsWorld.cpp
	physics/Joint.cpp
	physics/Raycast.cpp
)

set (PHYSICS_HEADERS
	physics/Actor.h
	physics/Controller.cpp
	physics/PhysicsTypes.h
	physics/PhysicsWorld.h
	physics/Joint.h
	physics/Raycast.h
	physics/PhysicsCallback.h
)

set (WORLD_SRC
	world/Camera.cpp
	world/SceneGraph.cpp
	world/SceneGraphManager.cpp
	world/Unit.cpp
	world/World.cpp
	world/WorldManager.cpp
)

set (WORLD_HEADERS
	world/Camera.h
	world/SceneGraph.h
	world/SceneGraphManager.h
	world/Unit.h
	world/World.h
	world/WorldManager.h
	world/WorldTypes.h
)

set (COMPILER_SRC
	compilers/BundleCompiler.cpp
)

set (COMPILER_HEADER
	compilers/BundleCompiler.h
)

set (GUI_SRC
	gui/Gui.cpp
)

set (GUI_HEADERS
	gui/Gui.h
	gui/GuiRect.h
	gui/GuiTriangle.h
	gui/GuiImage.h
	gui/GuiText.h
)

set (CROWN_LIBRARIES)

# Platform specific stuff
if (LINUX)
	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/engine/os/linux
		${CMAKE_SOURCE_DIR}/engine/renderers/backend/gl/glx
	)

	list (APPEND OS_HEADERS
		os/linux/OsWindow.h
		os/linux/OsFile.h
		os/linux/OsThread.h
		os/linux/Mutex.h
		os/linux/Cond.h
		os/posix/Semaphore.h
	)

	list (APPEND OS_SRC
		os/linux/Linux.cpp
		os/linux/OsWindow.cpp
		os/posix/OsFile.cpp
		os/posix/Posix.cpp
	)

	list (APPEND RENDERERS_SRC
		renderers/backend/gl/GLRenderer.cpp
		renderers/backend/gl/glx/GLContext.cpp
	)

	list (APPEND RENDERERS_HEADERS
		renderers/backend/gl/glx/GLContext.h
	)

	list (APPEND AUDIO_SRC
		audio/backend/ALSoundWorld.cpp
	)

	list (APPEND CROWN_LIBRARIES
		rt
		z
		X11
		Xrandr
		pthread
		dl
		GLEW
		GL
		openal
		vorbisfile
		luajit-5.1
	)

	set (COMPILER_FLAGS
		-pipe
		-std=c++03
		-ansi
		#-pedantic-errors
		-Wall
		-Wextra
		#-Werror
		-Wno-long-long
		-Wno-variadic-macros
		-Wno-unknown-pragmas
		-Wno-unused-parameter
		-Wno-unused-local-typedefs
		-fPIC
		#-fvisibility=hidden
		-fno-exceptions
	)

	if (CROWN_DEBUG)
		list (APPEND COMPILER_FLAGS -g -D_DEBUG)
		list (APPEND CROWN_LIBRARIES
			LowLevelCHECKED
			LowLevelClothCHECKED
			PhysX3CharacterKinematicCHECKED
			PhysX3CHECKED
			PhysX3CommonCHECKED
			PhysX3CookingCHECKED
			PhysX3ExtensionsCHECKED
			PhysXProfileSDKCHECKED
			PhysXVisualDebuggerSDKCHECKED
			PvdRuntimeCHECKED
			PxTaskCHECKED
			SceneQueryCHECKED
			SimulationControllerCHECKED
		)
	elseif (CROWN_DEVELOPMENT)
		list (APPEND COMPILER_FLAGS -O2 -DNDEBUG)
		list (APPEND CROWN_LIBRARIES
			LowLevelPROFILE
			LowLevelClothPROFILE
			PhysX3CharacterKinematicPROFILE
			PhysX3PROFILE
			PhysX3CommonPROFILE
			PhysX3CookingPROFILE
			PhysX3ExtensionsPROFILE
			PhysXProfileSDKPROFILE
			PhysXVisualDebuggerSDKPROFILE
			PvdRuntimePROFILE
			PxTaskPROFILE
			SceneQueryPROFILE
			SimulationControllerPROFILE
		)
	elseif (CROWN_RELEASE)
		list (APPEND COMPILER_FLAGS
			-DNDEBUG
			-O2
			-Wno-unused-variable
			-Wno-unused-parameter
			-Wno-maybe-uninitialized
			-Wno-unused-but-set-variable
			-Wno-unused-function
		)
		list (APPEND CROWN_LIBRARIES
			LowLevel
			LowLevelCloth
			PhysX3CharacterKinematic
			PhysX3
			PhysX3Common
			PhysX3Cooking
			PhysX3Extensions
			PhysXProfileSDK
			PhysXVisualDebuggerSDK
			PvdRuntime
			PxTask
			SceneQuery
			SimulationController
		)
	else ()
		message (FATAL_ERROR "Oops, you should not be here")
	endif (CROWN_DEBUG)

	set (CROWN_MAIN_SRC os/linux/main.cpp)
endif (LINUX)

if (WINDOWS)
	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/engine/os/win
		${CMAKE_SOURCE_DIR}/engine/renderers/backend/gl/wgl
	)

	list (APPEND OS_HEADERS
		os/win/OsFile.h
		os/win/OsThread.h
		os/win/OsSocket.h
		os/win/OsWindow.h
		os/win/Mutex.h
		os/win/Cond.h
		os/win/Semaphore.h
		os/win/AtomicInt.h
		os/win/inttypes.h
	)

	list (APPEND OS_SRC
		os/win/WinOS.cpp
		os/win/OsFile.cpp
		os/win/OsWindow.cpp
	)

	list (APPEND RENDERERS_SRC
		renderers/backend/gl/GLRenderer.cpp
		renderers/backend/gl/wgl/GLContext.cpp
	)

	list (APPEND RENDERERS_HEADERS
		renderers/backend/gl/wgl/GLContext.h
		renderers/backend/gl/wgl/wglext.h
	)

	list (APPEND AUDIO_SRC
		audio/backend/ALSoundWorld.cpp
	)

	list (APPEND CROWN_LIBRARIES
		zlib
		OpenGL32
		glew32
		lua51
		OpenAL32
		libvorbis
		libvorbisfile
	)

	set (COMPILER_FLAGS
	)
	
	if (CROWN_ARCH MATCHES "win32")
		set (WIN_PHYSXARCH x86)
	elseif (CROWN_ARCH MATCHES "win64")
		set (WIN_PHYSXARCH x64)
  else ()
    message (FATAL_ERROR "Oops, you should not be here")
	endif (CROWN_ARCH MATCHES "win32")

	if (CROWN_DEBUG)
		#list (APPEND COMPILER_FLAGS -g -D_DEBUG)
		list (APPEND CROWN_LIBRARIES
			#LowLevelCHECKED
			#LowLevelClothCHECKED
			PhysX3CharacterKinematicCHECKED_${WIN_PHYSXARCH}
			PhysX3CHECKED_${WIN_PHYSXARCH}
			PhysX3CommonCHECKED_${WIN_PHYSXARCH}
			PhysX3CookingCHECKED_${WIN_PHYSXARCH}
			PhysX3ExtensionsCHECKED
			PhysXProfileSDKCHECKED
			PhysXVisualDebuggerSDKCHECKED
			#PvdRuntimeCHECKED
			PxTaskCHECKED
			#SceneQueryCHECKED
			#SimulationControllerCHECKED
		)
	elseif (CROWN_DEVELOPMENT)
		#list (APPEND COMPILER_FLAGS -O2 -DNDEBUG)
		list (APPEND CROWN_LIBRARIES
			#LowLevelPROFILE
			#LowLevelClothPROFILE
			PhysX3CharacterKinematicPROFILE_${WIN_PHYSXARCH}
			PhysX3PROFILE_${WIN_PHYSXARCH}
			PhysX3CommonPROFILE_${WIN_PHYSXARCH}
			PhysX3CookingPROFILE_${WIN_PHYSXARCH}
			PhysX3ExtensionsPROFILE
			PhysXProfileSDKPROFILE
			PhysXVisualDebuggerSDKPROFILE
			#PvdRuntimePROFILE
			PxTaskPROFILE
			#SceneQueryPROFILE
			#SimulationControllerPROFILE
		)
	elseif (CROWN_RELEASE)
		list (APPEND COMPILER_FLAGS
			#-DNDEBUG
			#-O2
			#-Wno-unused-variable
			#-Wno-unused-parameter
			#-Wno-maybe-uninitialized
			#-Wno-unused-but-set-variable
			#-Wno-unused-function
		)
		list (APPEND CROWN_LIBRARIES
			#LowLevel
			#LowLevelCloth
			PhysX3CharacterKinematic_${WIN_PHYSXARCH}
			PhysX3_${WIN_PHYSXARCH}
			PhysX3Common_${WIN_PHYSXARCH}
			PhysX3Cooking_${WIN_PHYSXARCH}
			PhysX3Extensions
			PhysXProfileSDK
			PhysXVisualDebuggerSDK
			#PvdRuntime
			PxTask
			#SceneQuery
			#SimulationController
		)
	else ()
		message (FATAL_ERROR "Oops, you should not be here")
	endif (CROWN_DEBUG)

	set (CROWN_MAIN_SRC os/win/main.cpp)
endif(WINDOWS)

set (CROWN_SOURCES
	${SRC}
	${CORE_SRC}
	${CONTAINERS_SRC}
	${MATH_SRC}
	${FILESYSTEM_SRC}
	${JSON_SRC}
	${MEM_SRC}
	${COMPRESSORS_SRC}
	${SETTINGS_SRC}
	${INPUT_SRC}
	${RENDERERS_BACKEND_SRC}
	${RENDERERS_SRC}
	${RESOURCE_SRC}
	${OS_SRC}
	${LUA_SRC}
	${AUDIO_SRC}
	${PHYSICS_SRC}
	${WORLD_SRC}
	${COMPILER_SRC}
	${GUI_SRC}
)

set (CROWN_HEADERS
	${HEADERS}
	${CORE_HEADERS}
	${CONTAINERS_HEADERS}
	${MATH_HEADERS}
	${FILESYSTEM_HEADERS}
	${JSON_HEADERS}
	${MEM_HEADERS}
	${COMPRESSORS_HEADERS}
	${SETTINGS_HEADERS}
	${INPUT_HEADERS}
	${RENDERERS_BACKEND_HEADERS}
	${RENDERERS_HEADERS}
	${RESOURCE_HEADERS}
	${OS_HEADERS}
	${LUA_HEADERS}
	${AUDIO_HEADERS}
	${PHYSICS_HEADERS}
	${WORLD_HEADERS}
	${COMPILER_HEADERS}
	${GUI_HEADERS}
)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)

include_directories(${CROWN_INCLUDES} ${CROWN_THIRD_INCLUDES})
link_directories(${CROWN_THIRD_LIBS})
link_libraries(${CROWN_LIBRARIES})

add_definitions(${COMPILER_FLAGS})

add_library(crown-lib SHARED ${CROWN_SOURCES} ${CROWN_HEADERS})
set_target_properties (crown-lib PROPERTIES OUTPUT_NAME ${CROWN_LIBRARY_NAME})

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN")
add_executable(${CROWN_EXECUTABLE_NAME} ${CROWN_MAIN_SRC})
target_link_libraries(${CROWN_EXECUTABLE_NAME} crown-lib)

if (CROWN_BUILD_TESTS)
	add_subdirectory(tests)
endif (CROWN_BUILD_TESTS)


install (TARGETS crown-lib DESTINATION bin)
install (TARGETS crown-lib DESTINATION test)
install (TARGETS ${CROWN_EXECUTABLE_NAME} DESTINATION bin)

#install (FILES ${HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME})

#install (FILES ${CORE_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core)
#install (FILES ${BV_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/bv)
#install (FILES ${CONTAINERS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/containers)
#install (FILES ${MATH_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/math)
#install (FILES ${FILESYSTEM_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/filesystem)
#install (FILES ${MEM_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/mem)
#install (FILES ${COMPRESSORS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/compressors)
#install (FILES ${THREADS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/threads)
#install (FILES ${SETTINGS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/settings)
#
#install (FILES ${INPUT_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/input)
#install (FILES ${RENDERERS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/renderers)
#install (FILES ${OS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/os)
#install (FILES ${LUA_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/lua)
