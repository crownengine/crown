--
-- Copyright (c) 2012-2021 Daniele Bartolini et al.
-- License: https://github.com/dbartolini/crown/blob/master/LICENSE
--

local BULLET_DIR = (CROWN_DIR .. "3rdparty/bullet3/")

project "bullet"
	kind "StaticLib"
	language "C++"

	includedirs {
		BULLET_DIR .. "src",
	}

	configuration {}

	defines {
		"BT_THREADSAFE=0",
		"BT_USE_TBB=0",
		"BT_USE_PPL=0",
		"BT_USE_OPENMP=0",
		"B3_DBVT_BP_SORTPAIRS=0",
		"DBVT_BP_SORTPAIRS=0",
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
			"-Wno-maybe-uninitialized",
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
		BULLET_DIR .. "src/BulletCollision/**.cpp",
		BULLET_DIR .. "src/BulletDynamics/**.cpp",
		BULLET_DIR .. "src/BulletSoftBody/*.cpp",
		BULLET_DIR .. "src/LinearMath/**.cpp",
	}

	removefiles {
		BULLET_DIR .. "src/BulletCollision/Gimpact/**.h",
		BULLET_DIR .. "src/BulletCollision/Gimpact/**.cpp",
		BULLET_DIR .. "src/BulletDynamics/Vehicle/**.h",
		BULLET_DIR .. "src/BulletDynamics/Vehicle/**.cpp",
		BULLET_DIR .. "src/BulletSoftBody/**.h",
		BULLET_DIR .. "src/BulletSoftBody/**.cpp",
	}

	configuration {}
