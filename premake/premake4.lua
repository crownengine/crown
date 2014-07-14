-- Copyright (c) 2013 - 2014 Daniele Bartolini, Michele Rossi
-- Copyright (c) 2012 Daniele Bartolini, Simone Boscaratto
-- 
-- Permission is hereby granted, free of charge, to any person
-- obtaining a copy of this software and associated documentation
-- files (the "Software"), to deal in the Software without
-- restriction, including without limitation the rights to use,
-- copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the
-- Software is furnished to do so, subject to the following
-- conditions:
-- 
-- The above copyright notice and this permission notice shall be
-- included in all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
-- EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
-- OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
-- NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
-- HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
-- WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
-- OTHER DEALINGS IN THE SOFTWARE.

-- Globals
CROWN_SOURCE_DIR = path.getabsolute("..") .. "/"
CROWN_THIRD_DIR = CROWN_SOURCE_DIR .. "third/"
CROWN_BUILD_DIR = "../.build/"
CROWN_INSTALL_DIR = "../.installation/"
CROWN_ARCH = nil


solution "crown"
	configurations { "debug", "development", "release" }
	platforms { "x32", "x64" }

	configuration { "debug" }
		targetsuffix "-debug"
		defines { "_DEBUG", "CROWN_DEBUG" }

	configuration { "development" }
		flags { "OptimizeSpeed" }
		targetsuffix "-development"
		defines { "NDEBUG", "CROWN_DEVELOPMENT" }

	configuration { "release" }
		flags { "OptimizeSpeed" }
		targetsuffix "-release"
		defines { "NDEBUG", "CROWN_RELEASE" }

	if os.is("linux") and os.is64bit() then
		CROWN_ARCH = "x86_64"
	elseif os.is("linux") and not os.is64bit() then
		CROWN_ARCH = "x86_32"
	end


	-- Avoid error invoking premake4 --help
	if _ACTION == nil then return end

	if _ACTION == "clean" then os.rmdir(CROWN_BUILD_DIR) end

	if _ACTION == "gmake" then

	end


	project "libcrown"
		kind "SharedLib"
		language "C++"
		location(".build")
		targetdir(".installation")
		targetprefix ("")

		buildoptions {
			"-std=c++03",
			"-O0"
		}
		linkoptions { 
			"-Wl,--start-group $(addprefix -l, LowLevelClothCHECKED PhysX3CHECKED PhysX3CommonCHECKED PxTaskCHECKED LowLevelCHECKED PhysX3CharacterKinematicCHECKED PhysX3CookingCHECKED PhysX3ExtensionsCHECKED PhysX3VehicleCHECKED PhysXProfileSDKCHECKED PhysXVisualDebuggerSDKCHECKED PvdRuntimeCHECKED SceneQueryCHECKED SimulationControllerCHECKED) -Wl,--end-group"
		}

		links {
			"Xrandr",
			"pthread",
			"dl",
			"GL",
			"X11",
			"openal",
			"luajit-5.1",
		}

		flags {
			"StaticRuntime",
			"NoMinimalRebuild",
			"NoPCH",
			"NativeWChar",
			"NoRTTI",
			"NoExceptions",
			"NoEditAndContinue",
			"Symbols",
		}

		includedirs {
			CROWN_SOURCE_DIR .. "/engine",
			CROWN_SOURCE_DIR .. "/engine/core",
			CROWN_SOURCE_DIR .. "/engine/core/bv",
			CROWN_SOURCE_DIR .. "/engine/core/containers",
			CROWN_SOURCE_DIR .. "/engine/core/math",
			CROWN_SOURCE_DIR .. "/engine/core/mem",
			CROWN_SOURCE_DIR .. "/engine/core/compressors",
			CROWN_SOURCE_DIR .. "/engine/core/filesystem",
			CROWN_SOURCE_DIR .. "/engine/core/json",
			CROWN_SOURCE_DIR .. "/engine/core/strings",
			CROWN_SOURCE_DIR .. "/engine/core/settings",
			CROWN_SOURCE_DIR .. "/engine/os",
			CROWN_SOURCE_DIR .. "/engine/input",
			CROWN_SOURCE_DIR .. "/engine/renderers",
			CROWN_SOURCE_DIR .. "/engine/renderers/backend",
			CROWN_SOURCE_DIR .. "/engine/resource",
			CROWN_SOURCE_DIR .. "/engine/rpc",
			CROWN_SOURCE_DIR .. "/engine/network",
			CROWN_SOURCE_DIR .. "/engine/lua",
			CROWN_SOURCE_DIR .. "/engine/audio",
			CROWN_SOURCE_DIR .. "/engine/compilers",
			CROWN_SOURCE_DIR .. "/engine/physics",
			CROWN_SOURCE_DIR .. "/engine/world",
			CROWN_SOURCE_DIR .. "/engine/os/linux",
			CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/glx",

			CROWN_THIRD_DIR .. "opengl",
			CROWN_THIRD_DIR .. "openal/include",
			CROWN_THIRD_DIR .. "luajit/" .. CROWN_ARCH .. "/include/luajit-2.0",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/common",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/characterkinematic",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/cloth",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/common",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/cooking",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/extensions",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/foundation",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/geometry",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/particles",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/physxprofilesdk",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/physxvisualdebuggersdk",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/pvd",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/pxtask",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/RepX",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/RepXUpgrader",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/vehicle",
			CROWN_THIRD_DIR .. "freetype",
			CROWN_THIRD_DIR .. "stb_image",
			CROWN_THIRD_DIR .. "stb_vorbis"
		}

		libdirs {
			CROWN_THIRD_DIR .. "luajit/" .. CROWN_ARCH .. "/lib",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/lib"
		}

		files {
			-- Engine
			CROWN_SOURCE_DIR .. "engine/Config.h",
			CROWN_SOURCE_DIR .. "engine/Crown.h",
			CROWN_SOURCE_DIR .. "engine/Device.h",
			CROWN_SOURCE_DIR .. "engine/ConsoleServer.h",
			CROWN_SOURCE_DIR .. "engine/Device.cpp",
			CROWN_SOURCE_DIR .. "engine/ConsoleServer.cpp",

			-- Core
			CROWN_SOURCE_DIR .. "engine/core/Args.h",
			CROWN_SOURCE_DIR .. "engine/core/Assert.h",
			CROWN_SOURCE_DIR .. "engine/core/Log.h",
			CROWN_SOURCE_DIR .. "engine/core/Macros.h",
			CROWN_SOURCE_DIR .. "engine/core/Profiler.h",
			CROWN_SOURCE_DIR .. "engine/core/Types.h",

			-- Containers
			CROWN_SOURCE_DIR .. "engine/core/containers/Array.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/ContainerTypes.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/EventStream.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/Hash.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/IdArray.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/IdTable.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/Map.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/PriorityQueue.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/Queue.h",
			CROWN_SOURCE_DIR .. "engine/core/containers/Vector.h",

			-- Math
			CROWN_SOURCE_DIR .. "engine/core/math/Color4.cpp",
			CROWN_SOURCE_DIR .. "engine/core/math/AABB.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Color4.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Frustum.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Intersection.h",
			CROWN_SOURCE_DIR .. "engine/core/math/MathTypes.h",
			CROWN_SOURCE_DIR .. "engine/core/math/MathUtils.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Matrix3x3.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Matrix4x4.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Plane.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Quaternion.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Random.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Sphere.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Vector2.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Vector3.h",
			CROWN_SOURCE_DIR .. "engine/core/math/Vector4.h",

			-- Filesystem
			CROWN_SOURCE_DIR .. "engine/core/filesystem/DiskFile.cpp",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/DiskFilesystem.cpp",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/File.cpp",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/NetworkFile.cpp",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/NetworkFilesystem.cpp",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/DiskFile.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/DiskFilesystem.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/File.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/Filesystem.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/FilesystemTypes.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/NetworkFile.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/NetworkFilesystem.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/NullFile.h",
			CROWN_SOURCE_DIR .. "engine/core/filesystem/ReaderWriter.h",

			-- Json
			CROWN_SOURCE_DIR .. "engine/core/json/JSON.cpp",
			CROWN_SOURCE_DIR .. "engine/core/json/JSONParser.cpp",
			CROWN_SOURCE_DIR .. "engine/core/json/JSON.h",
			CROWN_SOURCE_DIR .. "engine/core/json/JSONParser.h",

			-- String
			CROWN_SOURCE_DIR .. "engine/core/strings/Path.cpp",
			CROWN_SOURCE_DIR .. "engine/core/strings/Path.h",
			CROWN_SOURCE_DIR .. "engine/core/strings/StringUtils.h",
			CROWN_SOURCE_DIR .. "engine/core/strings/StringStream.h",
			CROWN_SOURCE_DIR .. "engine/core/strings/DynamicString.h",

			-- Mem
			CROWN_SOURCE_DIR .. "engine/core/mem/LinearAllocator.cpp",
			CROWN_SOURCE_DIR .. "engine/core/mem/Memory.cpp",
			CROWN_SOURCE_DIR .. "engine/core/mem/PoolAllocator.cpp",
			CROWN_SOURCE_DIR .. "engine/core/mem/ProxyAllocator.cpp",
			CROWN_SOURCE_DIR .. "engine/core/mem/StackAllocator.cpp",
			CROWN_SOURCE_DIR .. "engine/core/mem/Allocator.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/LinearAllocator.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/Memory.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/MemoryTypes.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/PoolAllocator.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/ProxyAllocator.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/StackAllocator.h",
			CROWN_SOURCE_DIR .. "engine/core/mem/TempAllocator.h",

			-- Compressors
			CROWN_SOURCE_DIR .. "engine/core/compressors/ZipCompressor.cpp",
			CROWN_SOURCE_DIR .. "engine/core/compressors/Compressor.h",
			CROWN_SOURCE_DIR .. "engine/core/compressors/ZipCompressor.h",

			-- Settings
			CROWN_SOURCE_DIR .. "engine/core/settings/IntSetting.cpp",
			CROWN_SOURCE_DIR .. "engine/core/settings/FloatSetting.cpp",
			CROWN_SOURCE_DIR .. "engine/core/settings/StringSetting.cpp",
			CROWN_SOURCE_DIR .. "engine/core/settings/IntSetting.h",
			CROWN_SOURCE_DIR .. "engine/core/settings/FloatSetting.h",
			CROWN_SOURCE_DIR .. "engine/core/settings/StringSetting.h",

			-- Input
			CROWN_SOURCE_DIR .. "engine/input/Keyboard.h",
			CROWN_SOURCE_DIR .. "engine/input/KeyCode.h",
			CROWN_SOURCE_DIR .. "engine/input/Mouse.h",
			CROWN_SOURCE_DIR .. "engine/input/Touch.h",

			-- Renderers
			CROWN_SOURCE_DIR .. "engine/renderers/backend/Renderer.h",
			CROWN_SOURCE_DIR .. "engine/renderers/backend/RenderContext.h",
			CROWN_SOURCE_DIR .. "engine/renderers/backend/PixelFormat.h",
			CROWN_SOURCE_DIR .. "engine/renderers/backend/VertexFormat.h",
			CROWN_SOURCE_DIR .. "engine/renderers/DebugLine.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/Gui.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/Material.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/Mesh.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/RenderWorld.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/Sprite.cpp",

			-- Resource
			CROWN_SOURCE_DIR .. "engine/resource/FileBundle.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/FontResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/LevelResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/LuaResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/MaterialResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/MeshResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/PackageResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/PhysicsResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/ResourceLoader.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/ResourceManager.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/ResourceRegistry.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/SoundResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/SpriteResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/TextureResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/UnitResource.cpp",
			CROWN_SOURCE_DIR .. "engine/resource/Bundle.h",
			CROWN_SOURCE_DIR .. "engine/resource/FontResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/FontResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/LevelResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/LuaResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/MaterialResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/MeshResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/PackageResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/PhysicsResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/Resource.h",
			CROWN_SOURCE_DIR .. "engine/resource/ResourceLoader.h",
			CROWN_SOURCE_DIR .. "engine/resource/ResourceManager.h",
			CROWN_SOURCE_DIR .. "engine/resource/ResourcePackage.h",
			CROWN_SOURCE_DIR .. "engine/resource/ResourceRegistry.h",
			CROWN_SOURCE_DIR .. "engine/resource/SoundResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/SpriteResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/TextureResource.h",
			CROWN_SOURCE_DIR .. "engine/resource/UnitResource.h",

			-- Os
			CROWN_SOURCE_DIR .. "engine/os/OS.h",
			CROWN_SOURCE_DIR .. "engine/os/OsEventQueue.h",
			CROWN_SOURCE_DIR .. "engine/os/NetAddress.h",
			CROWN_SOURCE_DIR .. "engine/os/OsTypes.h",
			CROWN_SOURCE_DIR .. "engine/os/ScopedMutex.h",

			-- Lua
			CROWN_SOURCE_DIR .. "engine/lua/LuaAccelerometer.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaActor.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaCamera.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaColor4.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaController.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaDebugLine.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaDevice.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaEnvironment.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaFloatSetting.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaGui.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaIntSetting.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaKeyboard.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaMath.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaMatrix4x4.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaMatrix4x4Box.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaMesh.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaMouse.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaPhysicsWorld.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaQuaternion.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaQuaternionBox.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaRaycast.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaResourcePackage.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaSoundWorld.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaSprite.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaStringSetting.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaSystem.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaTouch.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaUnit.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaVector2.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaVector2Box.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaVector3.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaVector3Box.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaWindow.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaWorld.cpp",
			CROWN_SOURCE_DIR .. "engine/lua/LuaEnvironment.h",
			CROWN_SOURCE_DIR .. "engine/lua/LuaStack.h",
			CROWN_SOURCE_DIR .. "engine/lua/LuaSystem.h",

			-- Audio
			CROWN_SOURCE_DIR .. "engine/audio/SoundWorld.h",

			-- Physics
			CROWN_SOURCE_DIR .. "engine/physics/Actor.cpp",
			CROWN_SOURCE_DIR .. "engine/physics/Controller.cpp",
			CROWN_SOURCE_DIR .. "engine/physics/PhysicsWorld.cpp",
			CROWN_SOURCE_DIR .. "engine/physics/Joint.cpp",
			CROWN_SOURCE_DIR .. "engine/physics/Raycast.cpp",
			CROWN_SOURCE_DIR .. "engine/physics/Actor.h",
			CROWN_SOURCE_DIR .. "engine/physics/Controller.cpp",
			CROWN_SOURCE_DIR .. "engine/physics/PhysicsTypes.h",
			CROWN_SOURCE_DIR .. "engine/physics/PhysicsWorld.h",
			CROWN_SOURCE_DIR .. "engine/physics/Joint.h",
			CROWN_SOURCE_DIR .. "engine/physics/Raycast.h",
			CROWN_SOURCE_DIR .. "engine/physics/PhysicsCallback.h",

			-- World
			CROWN_SOURCE_DIR .. "engine/world/Camera.cpp",
			CROWN_SOURCE_DIR .. "engine/world/SceneGraph.cpp",
			CROWN_SOURCE_DIR .. "engine/world/SceneGraphManager.cpp",
			CROWN_SOURCE_DIR .. "engine/world/Unit.cpp",
			CROWN_SOURCE_DIR .. "engine/world/World.cpp",
			CROWN_SOURCE_DIR .. "engine/world/WorldManager.cpp",
			CROWN_SOURCE_DIR .. "engine/world/Camera.h",
			CROWN_SOURCE_DIR .. "engine/world/SceneGraph.h",
			CROWN_SOURCE_DIR .. "engine/world/SceneGraphManager.h",
			CROWN_SOURCE_DIR .. "engine/world/Unit.h",
			CROWN_SOURCE_DIR .. "engine/world/World.h",
			CROWN_SOURCE_DIR .. "engine/world/WorldManager.h",
			CROWN_SOURCE_DIR .. "engine/world/WorldTypes.h",

			-- Compilers
			CROWN_SOURCE_DIR .. "engine/compilers/BundleCompiler.cpp",
			CROWN_SOURCE_DIR .. "engine/compilers/BundleCompiler.h",

			-- Platform dep stuff
			CROWN_SOURCE_DIR .. "engine/os/linux/OsWindow.h",
			CROWN_SOURCE_DIR .. "engine/os/linux/OsFile.h",
			CROWN_SOURCE_DIR .. "engine/os/linux/OsThread.h",
			CROWN_SOURCE_DIR .. "engine/os/linux/Mutex.h",
			CROWN_SOURCE_DIR .. "engine/os/linux/Cond.h",
			CROWN_SOURCE_DIR .. "engine/os/posix/Semaphore.h",
			CROWN_SOURCE_DIR .. "engine/os/linux/Linux.cpp",
			CROWN_SOURCE_DIR .. "engine/os/linux/OsWindow.cpp",
			CROWN_SOURCE_DIR .. "engine/os/posix/OsFile.cpp",
			CROWN_SOURCE_DIR .. "engine/os/posix/Posix.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/GLRenderer.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/GLContext.cpp",
			CROWN_SOURCE_DIR .. "engine/renderers/backend/gl/glx/GLContext.h",
			CROWN_SOURCE_DIR .. "engine/audio/backend/ALSoundWorld.cpp"
		}

	project "crown"
		kind "ConsoleApp"
		language "C++"
		location(".build")
		targetdir(".installation")

		includedirs {
			CROWN_SOURCE_DIR .. "/engine",
			CROWN_SOURCE_DIR .. "/engine/core",
			CROWN_SOURCE_DIR .. "/engine/core/bv",
			CROWN_SOURCE_DIR .. "/engine/core/containers",
			CROWN_SOURCE_DIR .. "/engine/core/math",
			CROWN_SOURCE_DIR .. "/engine/core/mem",
			CROWN_SOURCE_DIR .. "/engine/core/compressors",
			CROWN_SOURCE_DIR .. "/engine/core/filesystem",
			CROWN_SOURCE_DIR .. "/engine/core/json",
			CROWN_SOURCE_DIR .. "/engine/core/strings",
			CROWN_SOURCE_DIR .. "/engine/core/settings",
			CROWN_SOURCE_DIR .. "/engine/os",
			CROWN_SOURCE_DIR .. "/engine/input",
			CROWN_SOURCE_DIR .. "/engine/renderers",
			CROWN_SOURCE_DIR .. "/engine/renderers/backend",
			CROWN_SOURCE_DIR .. "/engine/resource",
			CROWN_SOURCE_DIR .. "/engine/rpc",
			CROWN_SOURCE_DIR .. "/engine/network",
			CROWN_SOURCE_DIR .. "/engine/lua",
			CROWN_SOURCE_DIR .. "/engine/audio",
			CROWN_SOURCE_DIR .. "/engine/compilers",
			CROWN_SOURCE_DIR .. "/engine/physics",
			CROWN_SOURCE_DIR .. "/engine/world",
			CROWN_SOURCE_DIR .. "/engine/os/linux",
			CROWN_SOURCE_DIR .. "/engine/renderers/backend/gl/glx",

			CROWN_THIRD_DIR .. "luajit/" .. CROWN_ARCH .. "/include/luajit-2.0",
			CROWN_THIRD_DIR .. "opengl",
			CROWN_THIRD_DIR .. "openal/include",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/common",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/characterkinematic",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/cloth",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/common",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/cooking",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/extensions",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/foundation",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/geometry",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/particles",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/physxprofilesdk",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/physxvisualdebuggersdk",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/pvd",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/pxtask",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/RepX",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/RepXUpgrader",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/include/vehicle",
			CROWN_THIRD_DIR .. "freetype",
			CROWN_THIRD_DIR .. "stb_image",
			CROWN_THIRD_DIR .. "stb_vorbis"
		}

		linkoptions { 
			"-Wl,-rpath=\\$$ORIGIN"
		}

		libdirs {
			CROWN_THIRD_DIR .. "luajit/" .. CROWN_ARCH .. "/lib",
			CROWN_THIRD_DIR .. "physx/" .. CROWN_ARCH .. "/lib"
		}

		files { CROWN_SOURCE_DIR .. "engine/os/linux/main.cpp" }

		links { 
			"libcrown",
			"Xrandr",
			"pthread",
			"dl",
			"GL",
			"X11",
			"openal",
			"luajit-5.1"
		}