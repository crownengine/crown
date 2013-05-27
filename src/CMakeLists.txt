set (SRC
	Camera.cpp
	Device.cpp
	MaterialResource.cpp
	ResourceManager.cpp
	Skybox.cpp

	TextureResource.cpp
	TextResource.cpp
	FontResource.cpp
	VertexShaderResource.cpp
	PixelShaderResource.cpp
	ArchiveResourceArchive.cpp
	FileResourceArchive.cpp

	FPSSystem.cpp
	JSONParser.cpp
)

set (HEADERS
	Camera.h
	Config.h
	Crown.h
	Device.h
	Glyph.h
	MaterialResource.h
	Resource.h
	ResourceManager.h
	ResourceArchive.h
	Skybox.h

	TextureResource.h
	TextResource.h
	FontResource.h
	VertexShaderResource.h
	PixelShaderResource.h
	ArchiveResourceArchive.h
	FileResourceArchive.h

	FPSSystem.h
	JSONParser.h
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
	core/containers/Dictionary.h
	core/containers/Generic.h
	core/containers/List.h
	core/containers/RBTree.h
	core/containers/IdTable.h
)

set (MATH_SRC
	core/math/Color4.cpp
	core/math/Mat3.cpp
	core/math/Mat4.cpp
	core/math/MathUtils.cpp
	core/math/Plane.cpp
	core/math/Point2.cpp
	core/math/Quat.cpp
	core/math/Shape.cpp
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
	core/math/Shape.h
	core/math/Triangle.h
	core/math/Vec2.h
	core/math/Vec3.h
	core/math/Vec4.h
)

set (FILESYSTEM_SRC
	core/filesystem/DiskFile.cpp
	core/filesystem/MemoryFile.cpp
	core/filesystem/File.cpp
	
	core/filesystem/BinaryReader.cpp
	core/filesystem/BinaryWriter.cpp
	core/filesystem/TextReader.cpp
	core/filesystem/TextWriter.cpp

	core/filesystem/Filesystem.cpp
)

set (FILESYSTEM_HEADERS
	core/filesystem/DiskFile.h
	core/filesystem/MemoryFile.h
	core/filesystem/NullFile.h
	core/filesystem/File.h

	core/filesystem/BinaryReader.h
	core/filesystem/BinaryWriter.h
	core/filesystem/TextReader.h
	core/filesystem/TextWriter.h

	core/filesystem/Filesystem.h
)

set (STRINGS_SRC
)

set (STRINGS_HEADERS
	core/strings/Path.h
	core/strings/String.h
	core/strings/Hash.h
)

set (MEM_SRC
	core/mem/MallocAllocator.cpp
	core/mem/ProxyAllocator.cpp
)

set (MEM_HEADERS
	core/mem/Memory.h
	core/mem/Allocator.h
	core/mem/MallocAllocator.h
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
)

set (SETTINGS_HEADERS
	core/settings/IntSetting.h
	core/settings/FloatSetting.h
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
	renderers/PixelFormat.cpp
	renderers/VertexFormat.cpp
)

set (RENDERERS_HEADERS
	renderers/Renderer.h
	renderers/IndexBuffer.h
	renderers/VertexBuffer.h
	renderers/Material.h
	renderers/Texture.h
	renderers/DebugRenderer.h
	renderers/PixelFormat.h
	renderers/VertexFormat.h
)

set (OS_SRC
	os/OS.cpp
)

set (OS_HEADERS
	os/OS.h
	os/NetAddress.h
)

# Platform independent libraries
set (CROWN_LIBRARIES
	rt
	z
	luajit-5.1
)

set (CROWN_COMPILER_FLAGS
	-pipe
	-ansi
	-W
	-Wall
	-Wextra
	-Wno-long-long
	-Wno-variadic-macros
	-g
	-pg
)

# Platform specific stuff
if (LINUX)
	list (APPEND OS_HEADERS
		os/linux/OsWindow.h
		os/linux/TCPSocket.h
		os/linux/UDPSocket.h
		os/linux/OsFile.h
		os/linux/Thread.h
		os/linux/Mutex.h
		os/linux/Cond.h
	)

	list (APPEND OS_SRC
		os/linux/main.cpp
		os/linux/LinuxOS.cpp
		os/linux/OsWindow.cpp
		os/posix/TCPSocket.cpp
		os/posix/UDPSocket.cpp	
		os/posix/OsFile.cpp
		os/posix/Thread.cpp
		os/posix/Mutex.cpp
		os/posix/Cond.cpp
	)

	list (APPEND RENDERERS_SRC
		renderers/gl/GLRenderer.cpp
		renderers/gl/GLUtils.cpp
		renderers/gl/glx/GLContext.cpp
	)

	list (APPEND RENDERERS_HEADERS
		renderers/gl/GLRenderer.h
		renderers/gl/GLUtils.h
		renderers/gl/glx/GLContext.h
	)

	list (APPEND CROWN_LIBRARIES
		X11
		Xrandr
		pthread
		dl
		GLEW
		GL
	)

	set (CROWN_EXECUTABLE_NAME crown-linux)
	set (CROWN_MAIN_SRC os/linux/main.cpp)
endif (LINUX)

set (CROWN_SOURCES
	${SRC}
	${CORE_SRC}
	${BV_SRC}
	${CONTAINERS_SRC}
	${MATH_SRC}
	${FILESYSTEM_SRC}
	${MEM_SRC}
	${COMPRESSORS_SRC}
	${SETTINGS_SRC}

	${INPUT_SRC}

	${RENDERERS_SRC}

	${NETWORK_SRC}

	${OS_SRC}
)

set (CROWN_HEADERS
	${HEADERS}

	${CORE_HEADERS}
	${BV_HEADERS}
	${CONTAINERS_HEADERS}
	${MATH_HEADERS}
	${FILESYSTEM_HEADERS}
	${MEM_HEADERS}
	${COMPRESSORS_HEADERS}
	${SETTINGS_HEADERS}

	${INPUT_HEADERS}

	${RENDERERS_HEADERS}

	${NETWORK_HEADERS}

	${OS_HEADERS}

	${SCRIPT_HEADERS}
)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/Config.h)

include_directories(${CROWN_INCLUDES} ${CROWN_LUAJIT_INCLUDES})

link_directories(${CROWN_LUAJIT_LIBS})
link_libraries(${CROWN_LIBRARIES})

add_definitions(${CROWN_COMPILER_FLAGS})

add_library(crown SHARED ${CROWN_SOURCES} ${CROWN_HEADERS})
add_executable(${CROWN_EXECUTABLE_NAME} ${CROWN_MAIN_SRC})
target_link_libraries(${CROWN_EXECUTABLE_NAME} crown)

install (TARGETS crown DESTINATION lib/${CMAKE_PROJECT_NAME})
install (TARGETS ${CROWN_EXECUTABLE_NAME} DESTINATION bin)
install (FILES ${HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME})

install (FILES ${CORE_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core)
install (FILES ${BV_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/bv)
install (FILES ${CONTAINERS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/containers)
install (FILES ${MATH_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/math)
install (FILES ${FILESYSTEM_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/filesystem)
install (FILES ${MEM_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/mem)
install (FILES ${COMPRESSORS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/compressors)
install (FILES ${THREADS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/threads)
install (FILES ${SETTINGS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/core/settings)

install (FILES ${INPUT_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/input)
install (FILES ${RENDERERS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/renderers)
install (FILES ${NETWORK_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/network)
install (FILES ${OS_HEADERS} DESTINATION include/${CMAKE_PROJECT_NAME}/os)

