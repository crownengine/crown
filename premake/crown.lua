--
-- Copyright (c) 2012-2014 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "engine",
			CROWN_DIR .. "engine/core",
			CROWN_DIR .. "engine/core/containers",
			CROWN_DIR .. "engine/core/filesystem",
			CROWN_DIR .. "engine/core/json",
			CROWN_DIR .. "engine/core/math",
			CROWN_DIR .. "engine/core/memory",
			CROWN_DIR .. "engine/core/network",
			CROWN_DIR .. "engine/core/settings",
			CROWN_DIR .. "engine/core/strings",
			CROWN_DIR .. "engine/core/thread",
			CROWN_DIR .. "engine/main",
			CROWN_DIR .. "engine/input",
			CROWN_DIR .. "engine/renderers",
			CROWN_DIR .. "engine/resource",
			CROWN_DIR .. "engine/lua",
			CROWN_DIR .. "engine/audio",
			CROWN_DIR .. "engine/compilers",
			CROWN_DIR .. "engine/physics",
			CROWN_DIR .. "engine/world",
			CROWN_DIR .. "third/luajit/src",
			CROWN_DIR .. "third/openal/include",
			CROWN_DIR .. "third/freetype",
			CROWN_DIR .. "third/stb_image",
			CROWN_DIR .. "third/stb_vorbis",
			CROWN_DIR .. "third/bgfx/src",
			CROWN_DIR .. "third/bgfx/include",
			CROWN_DIR .. "third/bx/include",
		}

		defines {
			_defines,
		}

		configuration { "debug" }
			flags {
				"Symbols"
			}
			defines {
				"_DEBUG",
				"CROWN_DEBUG"
			}

		configuration { "development" }
			flags {
				"Symbols"
			}
			defines {
				"_DEBUG",
				"CROWN_DEBUG"
			}

		configuration { "release" }
			defines {
				"NDEBUG"
			}

		configuration { "linux*" }
			includedirs {
				"$(PHYSX_SDK_LINUX)/Include",
				"$(PHYSX_SDK_LINUX)/Include/common",
				"$(PHYSX_SDK_LINUX)/Include/characterkinematic",
				"$(PHYSX_SDK_LINUX)/Include/cloth",
				"$(PHYSX_SDK_LINUX)/Include/common",
				"$(PHYSX_SDK_LINUX)/Include/cooking",
				"$(PHYSX_SDK_LINUX)/Include/extensions",
				"$(PHYSX_SDK_LINUX)/Include/foundation",
				"$(PHYSX_SDK_LINUX)/Include/geometry",
				"$(PHYSX_SDK_LINUX)/Include/particles",
				"$(PHYSX_SDK_LINUX)/Include/physxprofilesdk",
				"$(PHYSX_SDK_LINUX)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_LINUX)/Include/pvd",
				"$(PHYSX_SDK_LINUX)/Include/pxtask",
				"$(PHYSX_SDK_LINUX)/Include/RepX",
				"$(PHYSX_SDK_LINUX)/Include/RepXUpgrader",
				"$(PHYSX_SDK_LINUX)/Include/vehicle",
			}

		configuration { "android*" }
			includedirs {
				"$(PHYSX_SDK_ANDROID)/Include",
				"$(PHYSX_SDK_ANDROID)/Include/common",
				"$(PHYSX_SDK_ANDROID)/Include/characterkinematic",
				"$(PHYSX_SDK_ANDROID)/Include/cloth",
				"$(PHYSX_SDK_ANDROID)/Include/common",
				"$(PHYSX_SDK_ANDROID)/Include/cooking",
				"$(PHYSX_SDK_ANDROID)/Include/extensions",
				"$(PHYSX_SDK_ANDROID)/Include/foundation",
				"$(PHYSX_SDK_ANDROID)/Include/geometry",
				"$(PHYSX_SDK_ANDROID)/Include/particles",
				"$(PHYSX_SDK_ANDROID)/Include/physxprofilesdk",
				"$(PHYSX_SDK_ANDROID)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_ANDROID)/Include/pvd",
				"$(PHYSX_SDK_ANDROID)/Include/pxtask",
				"$(PHYSX_SDK_ANDROID)/Include/RepX",
				"$(PHYSX_SDK_ANDROID)/Include/RepXUpgrader",
				"$(PHYSX_SDK_ANDROID)/Include/vehicle",
			}

		configuration { "vs2012" }
			includedirs {
				"$(PHYSX_SDK_WINDOWS)/Include",
				"$(PHYSX_SDK_WINDOWS)/Include/common",
				"$(PHYSX_SDK_WINDOWS)/Include/characterkinematic",
				"$(PHYSX_SDK_WINDOWS)/Include/cloth",
				"$(PHYSX_SDK_WINDOWS)/Include/common",
				"$(PHYSX_SDK_WINDOWS)/Include/cooking",
				"$(PHYSX_SDK_WINDOWS)/Include/extensions",
				"$(PHYSX_SDK_WINDOWS)/Include/foundation",
				"$(PHYSX_SDK_WINDOWS)/Include/geometry",
				"$(PHYSX_SDK_WINDOWS)/Include/particles",
				"$(PHYSX_SDK_WINDOWS)/Include/physxprofilesdk",
				"$(PHYSX_SDK_WINDOWS)/Include/physxvisualdebuggersdk",
				"$(PHYSX_SDK_WINDOWS)/Include/pvd",
				"$(PHYSX_SDK_WINDOWS)/Include/pxtask",
				"$(PHYSX_SDK_WINDOWS)/Include/RepX",
				"$(PHYSX_SDK_WINDOWS)/Include/RepXUpgrader",
				"$(PHYSX_SDK_WINDOWS)/Include/vehicle",
				"$(DXSDK_DIR)/Include",
			}

		configuration {}

		files {
			CROWN_DIR .. "engine/**.h",
			CROWN_DIR .. "engine/**.cpp"
		}

		strip()

		configuration {} -- reset configuration
end
