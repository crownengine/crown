--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

function crown_project(_name, _kind, _defines)

	project ("crown" .. _name)
		kind (_kind)

		includedirs {
			CROWN_DIR .. "src",
			CROWN_DIR .. "src/core",
			CROWN_DIR .. "src/core/containers",
			CROWN_DIR .. "src/core/error",
			CROWN_DIR .. "src/core/filesystem",
			CROWN_DIR .. "src/core/json",
			CROWN_DIR .. "src/core/math",
			CROWN_DIR .. "src/core/memory",
			CROWN_DIR .. "src/core/network",
			CROWN_DIR .. "src/core/strings",
			CROWN_DIR .. "src/core/thread",
			CROWN_DIR .. "src/device",
			CROWN_DIR .. "src/lua",
			CROWN_DIR .. "src/resource",
			CROWN_DIR .. "src/world",
			CROWN_DIR .. "third/bgfx/include",
			CROWN_DIR .. "third/bx/include",
			CROWN_DIR .. "third/stb",
		}

		defines {
			_defines,
		}

		links {
			"bgfx",
		}

		if _OPTIONS["with-luajit"] then
			includedirs {
				CROWN_DIR .. "third/luajit/src",
			}
			configuration { "android-arm" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/android_arm"
				}
			configuration { "linux-* or android-arm" }
				links {
					"luajit"
				}
			configuration { "x32", "linux-*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/linux_x86"
				}
			configuration { "x64", "linux-*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/linux_x64"
				}

			configuration { "vs*"}
				links {
					"lua51"
				}
			configuration { "x32", "vs*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/win_x86"
				}
			configuration { "x64", "vs*" }
				libdirs {
					CROWN_DIR .. "third/luajit/pre/win_x64"
				}

			configuration {}
		end

		if _OPTIONS["with-openal"] then
			includedirs {
				CROWN_DIR .. "third/openal/include"
			}

			configuration {}
				links { "openal", }

			configuration {}
		end

		if _OPTIONS["with-bullet"] then
			includedirs {
				CROWN_DIR .. "third/bullet3/src",
				CROWN_DIR .. "third/bullet3/src/Bullet3Collision",
				CROWN_DIR .. "third/bullet3/src/Bullet3Collision/BroadPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3Collision/NarrowPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3Common",
				CROWN_DIR .. "third/bullet3/src/Bullet3Dynamics",
				CROWN_DIR .. "third/bullet3/src/Bullet3Dynamics/ConstraintSover",
				CROWN_DIR .. "third/bullet3/src/Bullet3Geometry",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/BroadphaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/initialize",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/NarrowPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/ParallelPrimitives",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/Raycast",
				CROWN_DIR .. "third/bullet3/src/Bullet3OpenCL/RigidBody",
				CROWN_DIR .. "third/bullet3/src/Bullet3Serialize",
				CROWN_DIR .. "third/bullet3/src/Bullet3Serialize/Bullet2FileLoader",
				CROWN_DIR .. "third/bullet3/src/BulletCollision",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/BroadphaseCollision",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/CollisionDispatch",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/CollisionShapes",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/Gimpact",
				CROWN_DIR .. "third/bullet3/src/BulletCollision/NarrowPhaseCollision",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Character",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/ConstraintSolver",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Dynamics",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Featherstone",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/MLCPSolvers",
				CROWN_DIR .. "third/bullet3/src/BulletDynamics/Vehicle",
				CROWN_DIR .. "third/bullet3/src/BulletSoftBody",
				CROWN_DIR .. "third/bullet3/src/LinearMath",
				CROWN_DIR .. "third/bullet3/src/clew",
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
				"EGL",
				"GLESv2",
			}

		configuration { "linux-*" }
			links {
				"X11",
				"Xrandr",
				"pthread",
				"dl",
				"GL",
			}

		configuration { "vs*" }
			links {
				"OpenGL32",
				"dbghelp",
				"xinput",
				"psapi",
			}

		configuration {}

		files {
			CROWN_DIR .. "src/**.h",
			CROWN_DIR .. "src/**.cpp"
		}

		strip()

		configuration {} -- reset configuration
end
