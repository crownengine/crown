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
)

# architecture dependent libraries
set (CROWN_THIRD_LIBS
	${CROWN_THIRD}/luajit/lib
	${CROWN_THIRD}/glew-1.9.0/lib
	${CROWN_THIRD}/zlib/lib
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
	${CMAKE_SOURCE_DIR}/engine/resource
	${CMAKE_SOURCE_DIR}/engine/network
	${CMAKE_SOURCE_DIR}/engine/lua
	${CMAKE_SOURCE_DIR}/engine/compilers
	${CMAKE_SOURCE_DIR}/engine/compilers/lua
	${CMAKE_SOURCE_DIR}/engine/compilers/texture
	${CMAKE_SOURCE_DIR}/engine/compilers/mesh
	${CMAKE_SOURCE_DIR}/engine/compilers/package
)

set (SRC
	Camera.cpp
	Device.cpp
	EventBuffer.cpp
	ConsoleServer.cpp
	FPSSystem.cpp
)

set (HEADERS
	Camera.h
	Config.h
	Crown.h
	Device.h
	EventBuffer.h
	ConsoleServer.h
	FPSSystem.h
)

set (CORE_SRC
	core/Log.cpp
	core/Args.cpp
)

set (CORE_HEADERS
	core/Assert.h
	core/Types.h
	core/Args.h
	core/Log.h
)

set (BV_SRC
	core/bv/Circle.cpp
	core/bv/Frustum.cpp
	core/bv/Rect.cpp
)

set (BV_HEADERS
	core/bv/Box.h
	core/bv/Circle.h
	core/bv/Frustum.h
	core/bv/Rect.h
	core/bv/Sphere.h
)

set (CONTAINERS_SRC
	core/containers/Generic.cpp
)

set (CONTAINERS_HEADERS
	core/containers/Queue.h
	core/containers/PriorityQueue.h
	core/containers/Map.h
	core/containers/Generic.h
	core/containers/List.h
	core/containers/Vector.h
	core/containers/RBTree.h
	core/containers/IdTable.h
)

set (MATH_SRC
	core/math/Color4.cpp
	core/math/Mat3.cpp
	core/math/Mat4.cpp
	core/math/Plane.cpp
	core/math/Point2.cpp
	core/math/Quat.cpp
	core/math/Vec2.cpp
	core/math/Vec3.cpp
	core/math/Vec4.cpp
)

set (MATH_HEADERS
	core/math/Color4.h
	core/math/Interpolation.h
	core/math/Intersection.h
	core/math/Mat3.h
	core/math/Mat4.h
	core/math/MathUtils.h
	core/math/Plane.h
	core/math/Point2.h
	core/math/Quat.h
	core/math/Random.h
	core/math/Ray.h
	core/math/Triangle.h
	core/math/Vec2.h
	core/math/Vec3.h
	core/math/Vec4.h
)

set (FILESYSTEM_SRC
	core/filesystem/File.cpp
	core/filesystem/DiskFile.cpp
	core/filesystem/DiskFilesystem.cpp
)

set (FILESYSTEM_HEADERS
	core/filesystem/File.h
	core/filesystem/NullFile.h
	core/filesystem/DiskFile.h
	core/filesystem/BinaryReader.h
	core/filesystem/BinaryWriter.h
	core/filesystem/TextReader.h
	core/filesystem/TextWriter.h
	core/filesystem/Filesystem.h
	core/filesystem/DiskFilesystem.h
)

set (JSON_SRC
	core/json/JSONParser.cpp
)

set (JSON_HEADERS
	core/json/JSONParser.h
)

set (STRINGS_SRC
)

set (STRINGS_HEADERS
	core/strings/Path.h
	core/strings/StringUtils.h
	core/strings/StringStream.h
	core/strings/Hash.h
	core/strings/DynamicString.h
)

set (MEM_SRC
	core/mem/Memory.cpp
	core/mem/HeapAllocator.cpp
	core/mem/LinearAllocator.cpp
	core/mem/StackAllocator.cpp
	core/mem/ProxyAllocator.cpp
)

set (MEM_HEADERS
	core/mem/Memory.h
	core/mem/Allocator.h
	core/mem/HeapAllocator.h
	core/mem/TempAllocator.h
	core/mem/LinearAllocator.h
	core/mem/StackAllocator.h
	core/mem/ProxyAllocator.h
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
	input/EventDispatcher.cpp
	input/InputManager.cpp
	input/Keyboard.cpp
	input/Mouse.cpp
	input/Touch.cpp
	input/Accelerometer.cpp
)

set (INPUT_HEADERS
	input/EventDispatcher.h
	input/InputManager.h
	input/Keyboard.h
	input/KeyCode.h
	input/Mouse.h
	input/Touch.h
)

set (NETWORK_SRC
	network/BitMessage.cpp
)

set (NETWORK_HEADERS
	network/BitMessage.h
)

set (RENDERERS_SRC
	renderers/DebugRenderer.cpp
)

set (RENDERERS_HEADERS
	renderers/Renderer.h
	renderers/RenderContext.h
	renderers/DebugRenderer.h
	renderers/PixelFormat.h
	renderers/VertexFormat.h
)

set (RESOURCE_SRC
	resource/ResourceLoader.cpp
	resource/ResourceManager.cpp
	resource/ResourceRegistry.cpp
)

if (CROWN_DEBUG OR CROWN_DEVELOPMENT)
	list (APPEND RESOURCE_SRC resource/FileBundle.cpp)
elseif (CROWN_RELEASE)
	list (APPEND RESOURCE_SRC resource/ArchiveBundle.cpp)
else ()
	message (FATAL_ERROR "Oops, you should not be here")
endif (CROWN_DEBUG OR CROWN_DEVELOPMENT)

set (RESOURCE_HEADERS
	resource/Resource.h
	resource/ResourceFormat.h
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
	resource/ResourcePackage.h
)

set (OS_SRC
	os/OS.cpp
)

set (OS_HEADERS
	os/OS.h
	os/NetAddress.h
	os/OsEvents.h
)

set (LUA_SRC
	lua/LuaStack.cpp
	lua/LuaEnvironment.cpp
	lua/LuaIntSetting.cpp
	lua/LuaFloatSetting.cpp
	lua/LuaStringSetting.cpp
	lua/LuaVec2.cpp
	lua/LuaVec3.cpp
	lua/LuaMat4.cpp
	lua/LuaQuat.cpp
	lua/LuaMath.cpp
	lua/LuaMouse.cpp
	lua/LuaKeyboard.cpp
	lua/LuaAccelerometer.cpp
	lua/LuaDevice.cpp
	#lua/LuaWindow.cpp
	lua/LuaResourcePackage.cpp
)

set (LUA_HEADERS
	lua/LuaStack.h
	lua/LuaEnvironment.h
)

set (COMPILER_SRC
	compilers/Compiler.cpp
	compilers/BundleCompiler.cpp
	compilers/mesh/MeshCompiler.cpp
	compilers/lua/LuaCompiler.cpp
	compilers/texture/TextureCompiler.cpp
	compilers/package/PackageCompiler.cpp
)

set (COMPILER_HEADER
	compilers/Compiler.h
	compilers/BundleCompiler.h
	compilers/mesh/MeshCompiler.h
	compilers/lua/LuaCompiler.h
	compilers/texture/TextureCompiler.h
	compilers/package/PackageCompiler.h
)

set (CROWN_LIBRARIES)

# Platform specific stuff
if (LINUX)
	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/engine/os/linux
		${CMAKE_SOURCE_DIR}/engine/renderers/gl/glx
	)

	list (APPEND OS_HEADERS
		os/linux/OsWindow.h
		os/linux/TCPSocket.h
		os/linux/UDPSocket.h
		os/linux/OsFile.h
		os/linux/OsThread.h
		os/linux/Mutex.h
		os/linux/Cond.h
		os/posix/Semaphore.h
	)

	list (APPEND OS_SRC
		os/linux/LinuxOS.cpp
		os/linux/OsWindow.cpp
		os/posix/TCPSocket.cpp
		os/posix/UDPSocket.cpp	
		os/posix/OsFile.cpp
	)

	list (APPEND RENDERERS_SRC
		renderers/gl/GLRenderer.cpp
		renderers/gl/glx/GLContext.cpp
	)

	list (APPEND RENDERERS_HEADERS
		renderers/gl/GLRenderer.h
		renderers/gl/glx/GLContext.h
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
		luajit-5.1
	)

	set (COMPILER_FLAGS
		-pipe
		-std=c++03
		-ansi
		-pedantic-errors
		-Wall
		-Wextra
		-Werror
		-Wno-long-long
		-Wno-variadic-macros
		-fPIC
		#-fvisibility=hidden
	)

	if (CROWN_DEBUG)
		list (APPEND COMPILER_FLAGS -g -pg)
	elseif (CROWN_DEVELOPMENT)
		list (APPEND COMPILER_FLAGS -O2)
	elseif (CROWN_RELEASE)
		list (APPEND COMPILER_FLAGS
			-O2
			-Wno-unused-variable
			-Wno-unused-parameter
			-Wno-maybe-uninitialized
			-Wno-unused-but-set-variable
			-Wno-unused-function
		)
	else ()
		message (FATAL_ERROR "Oops, you should not be here")
	endif (CROWN_DEBUG)

	set (CROWN_MAIN_SRC os/linux/main.cpp)
endif (LINUX)

if (WINDOWS)
	list (APPEND CROWN_INCLUDES
		${CMAKE_SOURCE_DIR}/engine/os/win
		${CMAKE_SOURCE_DIR}/engine/renderers/gl/wgl
	)

	list (APPEND OS_HEADERS
		os/win/OsFile.h
		os/win/OsThread.h
		os/win/Mutex.h
		os/win/Cond.h
		os/win/Semaphore.h
		os/win/inttypes.h
	)

	list (APPEND OS_SRC
		os/win/WinOS.cpp
		os/win/OsFile.cpp
	)

	list (APPEND RENDERERS_SRC
		renderers/gl/GLRenderer.cpp
		renderers/gl/wgl/GLContext.cpp
	)

	list (APPEND RENDERERS_HEADERS
		renderers/gl/GLRenderer.h
		renderers/gl/wgl/GLContext.h
		renderers/gl/wgl/wglext.h
	)

	list (APPEND CROWN_LIBRARIES
		zlib
		OpenGL32
		glew32
		lua51
	)

	set (COMPILER_FLAGS
	)

	set (CROWN_MAIN_SRC os/win/main.cpp)
endif(WINDOWS)

set (CROWN_SOURCES
	${SRC}
	${CORE_SRC}
	${BV_SRC}
	${CONTAINERS_SRC}
	${MATH_SRC}
	${FILESYSTEM_SRC}
	${JSON_SRC}
	${MEM_SRC}
	${COMPRESSORS_SRC}
	${SETTINGS_SRC}
	${INPUT_SRC}
	${RENDERERS_SRC}
	${RESOURCE_SRC}
#	${NETWORK_SRC}
	${OS_SRC}
	${LUA_SRC}
	${COMPILER_SRC}
)

set (CROWN_HEADERS
	${HEADERS}
	${CORE_HEADERS}
	${BV_HEADERS}
	${CONTAINERS_HEADERS}
	${MATH_HEADERS}
	${FILESYSTEM_HEADERS}
	${JSON_HEADERS}
	${MEM_HEADERS}
	${COMPRESSORS_HEADERS}
	${SETTINGS_HEADERS}
	${INPUT_HEADERS}
	${RENDERERS_HEADERS}
	${RESOURCE_HEADERS}
	${RPC_HEADERS}
#	${NETWORK_HEADERS}
	${OS_HEADERS}
	${LUA_HEADERS}
	${COMPILER_HEADERS}
)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)

include_directories(${CROWN_INCLUDES} ${CROWN_THIRD_INCLUDES})
link_directories(${CROWN_THIRD_LIBS})
link_libraries(${CROWN_LIBRARIES})

add_definitions(${COMPILER_FLAGS})

add_library(crown SHARED ${CROWN_SOURCES} ${CROWN_HEADERS})

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}:$ORIGIN")
add_executable(${CROWN_EXECUTABLE_NAME} ${CROWN_MAIN_SRC})
target_link_libraries(${CROWN_EXECUTABLE_NAME} crown)


if (CROWN_BUILD_TESTS)
	#add_subdirectory(tests)
endif (CROWN_BUILD_TESTS)

install (TARGETS crown DESTINATION bin)
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
#install (FILES ${NETWORK_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/network)
#install (FILES ${OS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/os)
#install (FILES ${LUA_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/lua)
