--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

local BULLET_DIR = (CROWN_DIR .. "third/bullet3/")

project "bullet"
	kind "StaticLib"
	language "C++"

	includedirs {
		BULLET_DIR .. "src",
	}

	configuration { "linux-*" }
		buildoptions {
			"-Wno-unused-parameter",
			"-Wno-unused-variable",
			"-Wno-unused-but-set-variable",
			"-Wno-unused-function",
			"-Wno-sign-compare",
			"-Wno-type-limits",
			"-Wno-parentheses",
		}
		buildoptions_cpp {
			"-Wno-reorder",
		}

	configuration { "vs*" }
		buildoptions {
			"/wd4267",
			"/wd4244",
			"/wd4305",
		}

	configuration {}

	files {
		BULLET_DIR .. "src/*.cpp",
		BULLET_DIR .. "src/BulletCollision/**.cpp",
		BULLET_DIR .. "src/BulletDynamics/**.cpp",
		BULLET_DIR .. "src/BulletSoftBody/*.cpp",
		BULLET_DIR .. "src/LinearMath/**.cpp",
		BULLET_DIR .. "src/Bullet3Collision/**.cpp",
		BULLET_DIR .. "src/Bullet3Common/**.cpp",
		BULLET_DIR .. "src/Bullet3Dynamics/**.cpp",
		BULLET_DIR .. "src/Bullet3Geometry/**.cpp",
		BULLET_DIR .. "src/Bullet3Serialize/**.cpp",
	}

	configuration {}
