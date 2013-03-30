set (SRC
	Camera.cpp
	Device.cpp
	EventBuffer.cpp
	Filesystem.cpp
	Font.cpp
	Image.cpp
	Log.cpp
	Material.cpp
	MovableCamera.cpp
	Pixel.cpp
	ResourceManager.cpp
	ResourceLoader.cpp
	ResourceArchive.cpp
	Skybox.cpp
	Terrain.cpp
	TextRenderer.cpp
	World.cpp
	TextureResource.cpp
	
	TextResource.cpp
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
	Material.h
	MovableCamera.h
	Pixel.h
	Resource.h
	ResourceManager.h
	ResourceLoader.h
	ResourceArchive.h
	Skybox.h
	Terrain.h
	TextRenderer.h
	TextureResource.h
	World.h
	
	TextResource.h
)

set (CORE_SRC
)

set (CORE_HEADERS
	core/CoreEventArgs.h
	core/Delegate.h
	core/Exceptions.h
	core/SignalSlot.h
	core/Types.h
	core/VertexData.h
	core/WithProperties.h
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
)

set (GL_SRC
	renderers/gl/GLIndexBuffer.cpp
	renderers/gl/GLOcclusionQuery.cpp
	renderers/gl/GLRenderer.cpp
	renderers/gl/GLTexture.cpp
	renderers/gl/GLTextureManager.cpp
	renderers/gl/GLUtils.cpp
	renderers/gl/GLVertexBuffer.cpp
)

set (GL_HEADERS
	renderers/gl/GLIndexBuffer.h
	renderers/gl/GLOcclusionQuery.h
	renderers/gl/GLRenderer.h
	renderers/gl/GLTexture.h
	renderers/gl/GLTextureManager.h
	renderers/gl/GLUtils.h
	renderers/gl/GLVertexBuffer.h
)

set (OS_HEADERS
	os/OS.h
)

set (OS_SRC
	os/OS.cpp
)

set (LINUX_SRC
#	EGLRenderWindow.cpp
	os/linux/GLXRenderWindow.cpp
	os/linux/LinuxOS.cpp
	os/linux/Input.cpp
	os/linux/LinuxTCPSocket.cpp
	os/linux/LinuxUDPSocket.cpp	
	os/linux/File.cpp
)

set (LINUX_HEADERS
	os/linux/File.h
#	os/linux/EGLRenderWindow.h
)

set (WIN_SRC
#	os/win/WinTCPSocket.cpp
#	os/win/WinUDPSocket.cpp
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
	${INPUT_SRC}
	${INPUT_HEADERS}

	${OS_SRC}
	${OS_HEADERS}
	${LINUX_SRC}
	${LINUX_HEADERS}
	${WIN_SRC}
	${NETWORK_SRC}
	${NETWORK_HEADERS}

	${RENDERERS_SRC}
	${RENDERERS_HEADERS}

	${GL_SRC}
	${GL_HEADERS}
)

link_libraries(X11 GL GLEW Xrandr rt z)
add_library(crown STATIC ${SOURCES})
