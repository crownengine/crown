--
-- Copyright (c) 2012-2026 Daniele Bartolini et al.
-- SPDX-License-Identifier: MIT
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "src",
			CROWN_DIR .. "3rdparty/bgfx/include",
			CROWN_DIR .. "3rdparty/bgfx/src",
			CROWN_DIR .. "3rdparty/bimg/include",
			CROWN_DIR .. "3rdparty/bx/include",
			CROWN_DIR .. "3rdparty/stb",
			CROWN_DIR .. "3rdparty/bullet3/src",
			CROWN_DIR .. "3rdparty/openal/include",
			CROWN_DIR .. "3rdparty/ufbx",
		}

		defines {
			_defines,
		}

		links {
			"bgfx",
			"bimg",
			"bx",
			"bullet",
		}

		if not _OPTIONS["no-lua"] then
			if not _OPTIONS["no-luajit"] then
				includedirs {
					CROWN_DIR .. "3rdparty/luajit/src",
				}

				configuration { "not vs*" }
					links {
						"luajit"
					}

				configuration { "vs*" }
					links {
						"lua51"
					}

				configuration {}
			else
				includedirs {
					CROWN_DIR .. "3rdparty/lua/src",
				}

				links { "lua" }

				defines {
					"CROWN_USE_LUAJIT=0",
				}
			end
		end

		configuration { "debug" }
			defines {
				"BX_CONFIG_DEBUG=1",
			}

		configuration { "debug or development" }
			defines {
				"CROWN_DEBUG=1"
			}

		configuration { "development" }
			defines {
				"CROWN_DEVELOPMENT=1"
			}

		configuration { "release or development" }
			defines {
				"BX_CONFIG_DEBUG=0",
			}

		configuration { "android*" }
			kind "ConsoleApp"
			targetextension ".so"
			linkoptions {
				"-shared"
			}
			links {
				"EGL",
				"GLESv2",
				"OpenSLES",
				"openal",
			}
			includedirs {
				CROWN_DIR .. "3rdparty/android",
			}

		configuration { "linux-*" }
			links {
				"pthread",
				"openal",
			}

		configuration { "wasm" }
			kind "ConsoleApp"
			targetextension ".js"
			linkoptions {
				"-pthread",               -- https://emscripten.org/docs/porting/pthreads.html#compiling-with-pthreads-enabled
				"-lopenal",
				"-s ABORTING_MALLOC=0",
				"-s PTHREAD_POOL_SIZE=8",
				"-s EXIT_RUNTIME=1",
				"-s FORCE_FILESYSTEM",    -- https://github.com/emscripten-core/emscripten/blob/f5a1faa6c8d84fd5365a178013ce982d9168f6df/tools/file_packager.py#L17
				"-s MAX_WEBGL_VERSION=2",
				"-s TOTAL_MEMORY=256MB",
				"-s STACK_SIZE=256KB",
				"-s GL_ENABLE_GET_PROC_ADDRESS",
				-- "-s SAFE_HEAP=1",
			}

		configuration { "vs* or mingw*" }
			links {
				"openal",
				"dbghelp",
				"psapi",
				"ws2_32",
				"ole32",
				"gdi32",
			}

		configuration {}

		files {
			CROWN_DIR .. "src/**.h",
			CROWN_DIR .. "src/**.cpp",
			CROWN_DIR .. "3rdparty/ufbx/ufbx.c"
		}

		strip()

		configuration {} -- reset configuration
end
