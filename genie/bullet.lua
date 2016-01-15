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
		BULLET_DIR .. "src/Bullet3OpenCL",
		BULLET_DIR .. "src/clew",
	}

	configuration { "linux-*" }
		defines { "B3_USE_CLEW" }
		files {
			BULLET_DIR .. "src/clew/clew.c",
			BULLET_DIR .. "src/clew/clew.h",
			BULLET_DIR .. "src/Bullet3OpenCL/**.cpp",
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
