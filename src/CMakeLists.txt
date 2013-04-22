set (SRC
	Camera.cpp
	Device.cpp
	EventBuffer.cpp
	Filesystem.cpp
	Font.cpp
	Image.cpp
	Log.cpp
	MaterialResource.cpp
	MovableCamera.cpp
	Pixel.cpp
	ResourceManager.cpp
	ResourceLoader.cpp
	Skybox.cpp
	Terrain.cpp
	World.cpp

	TextureResource.cpp
	
	TextResource.cpp
	ArchiveResourceArchive.cpp
	FileResourceArchive.cpp

	FPSSystem.cpp
	JSONParser.cpp
)

set (HEADERS
	Camera.h
	Config.h
	Crown.h
	Data.h
	Device.h
	EventBuffer.h
	Filesystem.h
	Font.h
	Glyph.h
	Image.h
	Log.h
	MaterialResource.h
	MovableCamera.h
	Pixel.h
	Resource.h
	ResourceManager.h
	ResourceLoader.h
	ResourceArchive.h
	Skybox.h
	Terrain.h
	TextureResource.h
	World.h
	
	TextResource.h
	ArchiveResourceArchive.h
	FileResourceArchive.h

	FPSSystem.h
	JSONParser.h
)

set (CORE_SRC
)

set (CORE_HEADERS
	core/Types.h
	core/Args.h
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

set (STREAMS_SRC
	core/streams/FileStream.cpp
	core/streams/MemoryStream.cpp
	core/streams/Stream.cpp
	
	core/streams/BinaryReader.cpp
	core/streams/BinaryWriter.cpp
	core/streams/TextReader.cpp
	core/streams/TextWriter.cpp
)

set (STREAMS_HEADERS
	core/streams/FileStream.h
	core/streams/MemoryStream.h
	core/streams/NullStream.h
	core/streams/Stream.h

	core/streams/BinaryReader.h
	core/streams/BinaryWriter.h
	core/streams/TextReader.h
	core/streams/TextWriter.h
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
)

set (MEM_HEADERS
	core/mem/Memory.h
	core/mem/Allocator.h
	core/mem/MallocAllocator.h
)

set (COMPRESSORS_SRC
	core/compressors/ZipCompressor.cpp
)

set (COMPRESSORS_HEADERS
	core/compressors/Compressor.h
	core/compressors/ZipCompressor.h
)

set (THREADS_HEADERS
	core/threads/Thread.h
	core/threads/Mutex.h
)

set (THREADS_SRC
	core/threads/Thread.cpp
	core/threads/Mutex.cpp
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
)

set (INPUT_HEADERS
	input/EventDispatcher.h
	input/InputManager.h
	input/Keyboard.h
	input/KeyCode.h
	input/Mouse.h
	input/Touch.h
)

set (RENDERERS_SRC
	renderers/Renderer.cpp
)

set (RENDERERS_HEADERS
	renderers/Renderer.h
	renderers/IndexBuffer.h
	renderers/VertexBuffer.h
	renderers/OcclusionQuery.h
	renderers/Material.h
	renderers/Texture.h
)

set (OS_HEADERS
	os/OS.h
)

set (OS_SRC
	os/OS.cpp
)

set (NETWORK_SRC
	network/BitMessage.cpp
	network/AsyncConnection.cpp
	network/PacketQueue.h
)

set (NETWORK_HEADERS
	network/BitMessage.h
	network/AsyncConnection.h
	network/PacketQueue.h
)

set (SOURCES
	${SRC}
	${INCLUDES}
	${CORE_SRC}
	${CORE_HEADERS}
	${BV_SRC}
	${BV_HEADERS}
	${CONTAINERS_SRC}
	${CONTAINERS_HEADERS}
	${MATH_SRC}
	${MATH_HEADERS}
	${STREAMS_SRC}
	${STREAMS_HEADERS}
	${MEM_SRC}
	${MEM_HEADERS}
	${COMPRESSORS_SRC}
	${COMPRESSORS_HEADERS}
	${THREADS_SRC}
	${THREADS_HEADERS}
	${SETTINGS_SRC}
	${SETTINGS_HEADERS}

	${INPUT_SRC}
	${INPUT_HEADERS}

	${RENDERERS_SRC}
	${RENDERERS_HEADERS}

	${NETWORK_SRC}
	${NETWORK_HEADERS}

	${OS_SRC}
	${OS_HEADERS}
)

link_libraries(rt z)
add_library(crown STATIC ${SOURCES})

if (CROWN_BUILD_OPENGL)
target_link_libraries(crown crown-gl)
endif (CROWN_BUILD_OPENGL)

if (CROWN_BUILD_OPENGLES)
target_link_libraries(crown crown-gles)
endif (CROWN_BUILD_OPENGLES)

if (LINUX)
target_link_libraries(crown crown-linux)
endif (LINUX)

if (WINDOWS)
target_link_libraries(crown crown-win)
endif (WINDOWS)

