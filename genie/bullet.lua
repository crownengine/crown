--
-- Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
-- License: https://github.com/taylor001/crown/blob/master/LICENSE
--

local BULLET_DIR = (CROWN_DIR .. "third/bullet3/")

project "bullet"
	kind "StaticLib"
	language "C++"

	configuration {}

	configuration { "development or release", "linux-*" }
		buildoptions {
			"-O2",
			"-fomit-frame-pointer",
		}

	configuration { "linux-*" }
		links { "dl" }
		defines { "B3_USE_CLEW" }

	includedirs {
		BULLET_DIR .. "src",
		BULLET_DIR .. "src/Bullet3OpenCL",
		BULLET_DIR .. "src/clew",
	}

	files {
		BULLET_DIR .. "src/*.cpp",
		BULLET_DIR .. "src/BulletCollision/*.cpp",
		BULLET_DIR .. "src/BulletCollision/BroadphaseCollision/*.cpp",
		BULLET_DIR .. "src/BulletCollision/CollisionDispatch/*.cpp",
		BULLET_DIR .. "src/BulletCollision/CollisionShapes/*.cpp",
		BULLET_DIR .. "src/BulletCollision/Gimpact/*.cpp",
		BULLET_DIR .. "src/BulletCollision/NarrowPhaseCollision/*.cpp",

		BULLET_DIR .. "src/BulletDynamics/Character/*.cpp",
		BULLET_DIR .. "src/BulletDynamics/ConstraintSolver/*.cpp",
		BULLET_DIR .. "src/BulletDynamics/Dynamics/*.cpp",
		BULLET_DIR .. "src/BulletDynamics/Featherstone/*.cpp",
		BULLET_DIR .. "src/BulletDynamics/MLCPSolvers/*.cpp",
		BULLET_DIR .. "src/BulletDynamics/Vehicle/*.cpp",

		BULLET_DIR .. "src/BulletSoftBody/*.cpp",

		BULLET_DIR .. "src/LinearMath/*.cpp",

		BULLET_DIR .. "src/Bullet3Collision/BroadphaseCollision/*.cpp",
		BULLET_DIR .. "src/Bullet3Collision/NarrowPhaseCollision/*.cpp",

		BULLET_DIR .. "src/Bullet3Common/*.cpp",
		BULLET_DIR .. "src/Bullet3Common/shared/*.cpp",

		BULLET_DIR .. "src/Bullet3Dynamics/*.cpp",
		BULLET_DIR .. "src/Bullet3Dynamics/ConstraintSolver/*.cpp",
		BULLET_DIR .. "src/Bullet3Dynamics/shared/*.cpp",

		BULLET_DIR .. "src/Bullet3Geometry/*.cpp",

		BULLET_DIR .. "src/Bullet3Serialize/**.cpp",

		BULLET_DIR .. "src/clew/clew.c",
		BULLET_DIR .. "src/clew/clew.h",

		BULLET_DIR .. "src/Bullet3OpenCL/**.cpp",
	}

	configuration {}
