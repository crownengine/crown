--
-- Copyright (c) 2012-2017 Daniele Bartolini and individual contributors.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "src",
			CROWN_DIR .. "3rdparty/bgfx/include",
			CROWN_DIR .. "3rdparty/bx/include",
			CROWN_DIR .. "3rdparty/stb",
		}

		defines {
			_defines,
		}

		links {
			"bgfx",
			"bimg",
			"bx",
		}

		if _OPTIONS["with-luajit"] then
			includedirs {
				CROWN_DIR .. "3rdparty/luajit/src",
			}

			configuration { "not vs*" }
				links {
					"luajit"
				}

			configuration { "vs*"}
				links {
					"lua51"
				}
			configuration { "x32", "vs*" }
				libdirs {
					CROWN_DIR .. "3rdparty/luajit/pre/win_x32"
				}
			configuration { "x64", "vs*" }
				libdirs {
					CROWN_DIR .. "3rdparty/luajit/pre/win_x64"
				}

			configuration {}
		end

		if _OPTIONS["with-openal"] then
			includedirs {
				CROWN_DIR .. "3rdparty/openal/include"
			}

			configuration {}
				links { "openal", }

			configuration {}
		end

		if _OPTIONS["with-bullet"] then
			includedirs {
				CROWN_DIR .. "3rdparty/bullet3/src",
			}

			links {
				"bullet",
			}

			configuration {}
		end

		configuration { "debug or development" }
			defines {
				"CROWN_DEBUG=1"
			}

		configuration { "development" }
			defines {
				"CROWN_DEVELOPMENT=1"
			}

		configuration { "android*" }
			kind "ConsoleApp"
			targetextension ".so"
			linkoptions {
				"-shared"
			}
			links {
				"gcc",
				"EGL",
				"GLESv2",
			}

		configuration { "linux-*" }
			links {
				"X11",
				"Xrandr",
				"pthread",
				"GL",
			}

		configuration { "vs* or mingw*" }
			links {
				"dbghelp",
				"xinput",
				"psapi",
				"ws2_32",
				"ole32",
				"gdi32",
			}

		configuration {}

		files {
			CROWN_DIR .. "src/**.h",
			CROWN_DIR .. "src/**.cpp"
		}

		strip()

		configuration {} -- reset configuration
end
