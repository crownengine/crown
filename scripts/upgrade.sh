#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2023 Daniele Bartolini et al.

update_luajit () {
	# Download latest luajit.
	rm -rf 3rdparty/luajit
	git clone https://github.com/LuaJIT/LuaJIT.git 3rdparty/luajit --branch=v2.1 --depth=1

	# Cleanup.
	rm -rf 3rdparty/luajit/.gitignore
	rm -rf 3rdparty/luajit/doc
	rm -rf 3rdparty/luajit/etc
	rm -rf 3rdparty/luajit/Makefile
	rm -rf 3rdparty/luajit/README

	# Bump affected resources versions.
	RESOURCE_TYPES_H=src/resource/types.h

	sed -Ei 's/(.*RESOURCE_VERSION_SCRIPT)(.*RESOURCE_VERSION\()([0-9]+)(.*)/echo "\1\2$((\3 + 1))\4"/ge' src/resource/types.h

	# Add changes and commit.
	git add -f 3rdparty/luajit
	git add "${RESOURCE_TYPES_H}"
	git commit -m "3rdparty: update luajit"
}

update_bgfx () {
	# Download latest bx.
	rm -rf 3rdparty/bx
	git clone https://github.com/bkaradzic/bx 3rdparty/bx --depth=1

	# Cleanup bx.
	rm -rf 3rdparty/bx/.appveyor.yml
	rm -rf 3rdparty/bx/.editorconfig
	rm -rf 3rdparty/bx/.git
	rm -rf 3rdparty/bx/.gitattributes
	rm -rf 3rdparty/bx/.gitignore
	rm -rf 3rdparty/bx/.travis.yml
	rm -rf 3rdparty/bx/3rdparty/catch/
	rm -rf 3rdparty/bx/makefile
	rm -rf 3rdparty/bx/README.md
	rm -rf 3rdparty/bx/scripts/bin2c.lua
	rm -rf 3rdparty/bx/scripts/lemon.lua
	rm -rf 3rdparty/bx/tests/
	rm -rf 3rdparty/bx/tools/bin2c/
	rm -rf 3rdparty/bx/tools/lemon/

	# Download latest bimg.
	rm -rf 3rdparty/bimg
	git clone https://github.com/bkaradzic/bimg 3rdparty/bimg --depth=1

	# Cleanup bimg.
	rm -rf 3rdparty/bimg/.git
	rm -rf 3rdparty/bimg/.appveyor.yml
	rm -rf 3rdparty/bimg/.editorconfig
	rm -rf 3rdparty/bimg/.gitattributes
	rm -rf 3rdparty/bimg/.gitignore
	rm -rf 3rdparty/bimg/.travis.yml
	rm -rf 3rdparty/bimg/makefile
	rm -rf 3rdparty/bimg/README.md

	# Download latest bgfx.
	rm -rf 3rdparty/bgfx
	git clone https://github.com/bkaradzic/bgfx 3rdparty/bgfx --depth=1

	# Regenerate samples/core/shaders/common.shader.
	COMMON_SHADER=samples/core/shaders/common.shader

	rm "${COMMON_SHADER}"
	{
		echo "sampler_states = {"
		echo "	clamp_point = {"
		echo "		wrap_u = \"clamp\""
		echo "		wrap_v = \"clamp\""
		echo "		wrap_w = \"clamp\""
		echo "		filter_min = \"point\""
		echo "		filter_mag = \"point\""
		echo "	}"
		echo ""
		echo "	clamp_anisotropic = {"
		echo "		wrap_u = \"clamp\""
		echo "		wrap_v = \"clamp\""
		echo "		wrap_w = \"clamp\""
		echo "		filter_min = \"anisotropic\""
		echo "		filter_mag = \"anisotropic\""
		echo "	}"
		echo ""
		echo "	mirror_point = {"
		echo "		wrap_u = \"mirror\""
		echo "		wrap_v = \"mirror\""
		echo "		wrap_w = \"mirror\""
		echo "		filter_min = \"point\""
		echo "		filter_mag = \"point\""
		echo "	}"
		echo ""
		echo "	mirror_anisotropic = {"
		echo "		wrap_u = \"mirror\""
		echo "		wrap_v = \"mirror\""
		echo "		wrap_w = \"mirror\""
		echo "		filter_min = \"anisotropic\""
		echo "		filter_mag = \"anisotropic\""
		echo "	}"
		echo "}"
		echo ""
		echo "bgfx_shaders = {"
		echo "	common = {"
		echo "		code = \"\"\""
		sed 's/^/\t\t\t/' 3rdparty/bgfx/src/bgfx_shader.sh 3rdparty/bgfx/examples/common/shaderlib.sh
		echo "		\"\"\""
		echo "	}"
		echo "}"
	} >> "${COMMON_SHADER}"

	# Remove trailing tabs.
	sed -i 's/\t*$//' "${COMMON_SHADER}"

	# Cleanup bgfx.
	rm -rf 3rdparty/bgfx/.appveyor.yml
	rm -rf 3rdparty/bgfx/.editorconfig
	rm -rf 3rdparty/bgfx/.git
	rm -rf 3rdparty/bgfx/.gitattributes
	rm -rf 3rdparty/bgfx/.github
	rm -rf 3rdparty/bgfx/.gitignore
	rm -rf 3rdparty/bgfx/.travis.yml
	rm -rf 3rdparty/bgfx/3rdparty/.editorconfig
	rm -rf 3rdparty/bgfx/3rdparty/cgltf/
	rm -rf 3rdparty/bgfx/3rdparty/dear-imgui/
	rm -rf 3rdparty/bgfx/3rdparty/freetype/
	rm -rf 3rdparty/bgfx/3rdparty/glslang/build
	rm -rf 3rdparty/bgfx/3rdparty/glslang/build_info.py
	rm -rf 3rdparty/bgfx/3rdparty/glslang/license-checker.cfg
	rm -rf 3rdparty/bgfx/3rdparty/iconfontheaders/
	rm -rf 3rdparty/bgfx/3rdparty/meshoptimizer/
	rm -rf 3rdparty/bgfx/3rdparty/sdf/
	rm -rf 3rdparty/bgfx/3rdparty/stb/
	rm -rf 3rdparty/bgfx/bindings/
	rm -rf 3rdparty/bgfx/CODEOWNERS
	rm -rf 3rdparty/bgfx/CONTRIBUTING.md
	rm -rf 3rdparty/bgfx/examples/
	rm -rf 3rdparty/bgfx/makefile
	rm -rf 3rdparty/bgfx/README.md
	rm -rf 3rdparty/bgfx/scripts/bgfx-codegen.lua
	rm -rf 3rdparty/bgfx/scripts/bgfx.doxygen
	rm -rf 3rdparty/bgfx/scripts/bgfx.idl
	rm -rf 3rdparty/bgfx/scripts/bindings-*.lua
	rm -rf 3rdparty/bgfx/scripts/build.ninja
	rm -rf 3rdparty/bgfx/scripts/codegen.lua
	rm -rf 3rdparty/bgfx/scripts/doxygen.lua
	rm -rf 3rdparty/bgfx/scripts/geometryc.lua
	rm -rf 3rdparty/bgfx/scripts/geometryv.lua
	rm -rf 3rdparty/bgfx/scripts/idl.lua
	rm -rf 3rdparty/bgfx/scripts/shader-embeded.mk
	rm -rf 3rdparty/bgfx/scripts/shader.mk
	rm -rf 3rdparty/bgfx/scripts/temp.bgfx.h
	rm -rf 3rdparty/bgfx/scripts/temp.bgfx.idl.inl
	rm -rf 3rdparty/bgfx/scripts/temp.defines.h
	rm -rf 3rdparty/bgfx/scripts/texturev.lua
	rm -rf 3rdparty/bgfx/scripts/tools.mk
	rm -rf 3rdparty/bgfx/src/bgfx_compute.sh
	rm -rf 3rdparty/bgfx/src/bgfx_shader.sh
	rm -rf 3rdparty/bgfx/src/fs_clear0.sc
	rm -rf 3rdparty/bgfx/src/fs_clear1.sc
	rm -rf 3rdparty/bgfx/src/fs_clear2.sc
	rm -rf 3rdparty/bgfx/src/fs_clear3.sc
	rm -rf 3rdparty/bgfx/src/fs_clear4.sc
	rm -rf 3rdparty/bgfx/src/fs_clear5.sc
	rm -rf 3rdparty/bgfx/src/fs_clear6.sc
	rm -rf 3rdparty/bgfx/src/fs_clear7.sc
	rm -rf 3rdparty/bgfx/src/fs_debugfont.sc
	rm -rf 3rdparty/bgfx/src/makefile
	rm -rf 3rdparty/bgfx/src/varying.def.sc
	rm -rf 3rdparty/bgfx/src/vs_clear.sc
	rm -rf 3rdparty/bgfx/src/vs_debugfont.sc
	rm -rf 3rdparty/bgfx/tools/geometryc/
	rm -rf 3rdparty/bgfx/tools/geometryv/
	rm -rf 3rdparty/bgfx/tools/texturev/

	# Bump affected resources versions.
	RESOURCE_TYPES_H=src/resource/types.h

	sed -Ei 's/(.*RESOURCE_VERSION_TEXTURE)(.*RESOURCE_VERSION\()([0-9]+)(.*)/echo "\1\2$((\3 + 1))\4"/ge' src/resource/types.h
	sed -Ei 's/(.*RESOURCE_VERSION_SHADER)(.*RESOURCE_VERSION\()([0-9]+)(.*)/echo "\1\2$((\3 + 1))\4"/ge' src/resource/types.h

	# Add changes and commit.
	git add -f 3rdparty/bx
	git add -f 3rdparty/bimg
	git add -f 3rdparty/bgfx
	git add "${COMMON_SHADER}"
	git add "${RESOURCE_TYPES_H}"
	git commit -m "3rdparty: update bx, bimg and bgfx"
}

update_bullet () {
	# Download latest bullet3.
	rm -rf 3rdparty/bullet3
	git clone https://github.com/bulletphysics/bullet3 3rdparty/bullet3 --depth=1

	# Cleanup.
	rm -rf 3rdparty/bullet3/.ci
	rm -rf 3rdparty/bullet3/.git
	rm -rf 3rdparty/bullet3/.github
	rm -rf 3rdparty/bullet3/.gitignore
	rm -rf 3rdparty/bullet3/.style.yapf
	rm -rf 3rdparty/bullet3/.travis.yml
	rm -rf 3rdparty/bullet3/_clang-format
	rm -rf 3rdparty/bullet3/appveyor.yml
	rm -rf 3rdparty/bullet3/AUTHORS.txt
	rm -rf 3rdparty/bullet3/build3
	rm -rf 3rdparty/bullet3/build_cmake_pybullet_double.sh
	rm -rf 3rdparty/bullet3/build_visual_studio_vr_pybullet_double.bat
	rm -rf 3rdparty/bullet3/build_visual_studio_vr_pybullet_double_cmake.bat
	rm -rf 3rdparty/bullet3/build_visual_studio_vr_pybullet_double_dynamic.bat
	rm -rf 3rdparty/bullet3/build_visual_studio_without_pybullet_vr.bat
	rm -rf 3rdparty/bullet3/bullet.pc.cmake
	rm -rf 3rdparty/bullet3/BulletConfig.cmake.in
	rm -rf 3rdparty/bullet3/clang-format-all.sh
	rm -rf 3rdparty/bullet3/CMakeLists.txt
	rm -rf 3rdparty/bullet3/data
	rm -rf 3rdparty/bullet3/docs
	rm -rf 3rdparty/bullet3/Doxyfile
	rm -rf 3rdparty/bullet3/examples
	rm -rf 3rdparty/bullet3/Extras
	rm -rf 3rdparty/bullet3/MANIFEST.in
	rm -rf 3rdparty/bullet3/README.md
	rm -rf 3rdparty/bullet3/setup.py
	rm -rf 3rdparty/bullet3/src/.DS_Store
	rm -rf 3rdparty/bullet3/test
	rm -rf 3rdparty/bullet3/UseBullet.cmake
	rm -rf 3rdparty/bullet3/VERSION
	rm -rf 3rdparty/bullet3/xcode.command
	find 3rdparty/bullet3 -type f -name 'CMakeLists.txt' -exec rm {} +
	find 3rdparty/bullet3 -type f -name 'premake4.lua' -exec rm {} +

	# Add changes and commit.
	git add -f 3rdparty/bullet3
	git commit -m "3rdparty: update bullet3"
}

update_openal () {
	# Backup generated headers.
	cp 3rdparty/openal/config.h /tmp/config.h
	cp 3rdparty/openal/version.h /tmp/version.h
	cp 3rdparty/openal/bsinc_inc.h /tmp/bsinc_inc.h

	# Download latest sources.
	rm -rf 3rdparty/openal
	git clone https://github.com/kcat/openal-soft 3rdparty/openal --depth=1

	# Cleanup.
	rm -rf 3rdparty/openal/.gitignore
	rm -rf 3rdparty/openal/.travis.yml
	rm -rf 3rdparty/openal/CMakeLists.txt
	rm -rf 3rdparty/openal/README.md
	rm -rf 3rdparty/openal/XCompile-Android.txt
	rm -rf 3rdparty/openal/XCompile.txt
	rm -rf 3rdparty/openal/alsoftrc.sample
	rm -rf 3rdparty/openal/appveyor.yml
	rm -rf 3rdparty/openal/build/.empty
	rm -rf 3rdparty/openal/cmake/FindALSA.cmake
	rm -rf 3rdparty/openal/cmake/FindAudioIO.cmake
	rm -rf 3rdparty/openal/cmake/FindDSound.cmake
	rm -rf 3rdparty/openal/cmake/FindFFmpeg.cmake
	rm -rf 3rdparty/openal/cmake/FindJACK.cmake
	rm -rf 3rdparty/openal/cmake/FindMySOFA.cmake
	rm -rf 3rdparty/openal/cmake/FindOSS.cmake
	rm -rf 3rdparty/openal/cmake/FindOpenSL.cmake
	rm -rf 3rdparty/openal/cmake/FindPortAudio.cmake
	rm -rf 3rdparty/openal/cmake/FindPulseAudio.cmake
	rm -rf 3rdparty/openal/cmake/FindQSA.cmake
	rm -rf 3rdparty/openal/cmake/FindSDL2.cmake
	rm -rf 3rdparty/openal/cmake/FindSDL_sound.cmake
	rm -rf 3rdparty/openal/cmake/FindSoundIO.cmake
	rm -rf 3rdparty/openal/cmake/FindWindowsSDK.cmake
	rm -rf 3rdparty/openal/config.h.in
	rm -rf 3rdparty/openal/docs/3D7.1.txt
	rm -rf 3rdparty/openal/docs/ambdec.txt
	rm -rf 3rdparty/openal/docs/ambisonics.txt
	rm -rf 3rdparty/openal/docs/env-vars.txt
	rm -rf 3rdparty/openal/docs/hrtf.txt
	rm -rf 3rdparty/openal/examples/alffplay.cpp
	rm -rf 3rdparty/openal/examples/alhrtf.c
	rm -rf 3rdparty/openal/examples/allatency.c
	rm -rf 3rdparty/openal/examples/alloopback.c
	rm -rf 3rdparty/openal/examples/almultireverb.c
	rm -rf 3rdparty/openal/examples/alplay.c
	rm -rf 3rdparty/openal/examples/alrecord.c
	rm -rf 3rdparty/openal/examples/alreverb.c
	rm -rf 3rdparty/openal/examples/alstream.c
	rm -rf 3rdparty/openal/examples/altonegen.c
	rm -rf 3rdparty/openal/examples/common/alhelpers.c
	rm -rf 3rdparty/openal/examples/common/alhelpers.h
	rm -rf 3rdparty/openal/native-tools/CMakeLists.txt
	rm -rf 3rdparty/openal/native-tools/bin2h.c
	rm -rf 3rdparty/openal/native-tools/bsincgen.c
	rm -rf 3rdparty/openal/openal.pc.in
	rm -rf 3rdparty/openal/resources/openal32.rc
	rm -rf 3rdparty/openal/resources/resource.h
	rm -rf 3rdparty/openal/resources/router.rc
	rm -rf 3rdparty/openal/resources/soft_oal.rc
	rm -rf 3rdparty/openal/router/al.cpp
	rm -rf 3rdparty/openal/router/alc.cpp
	rm -rf 3rdparty/openal/router/router.cpp
	rm -rf 3rdparty/openal/router/router.h
	rm -rf 3rdparty/openal/utils/CIAIR.def
	rm -rf 3rdparty/openal/utils/IRC_1005.def
	rm -rf 3rdparty/openal/utils/MIT_KEMAR.def
	rm -rf 3rdparty/openal/utils/MIT_KEMAR_sofa.def
	rm -rf 3rdparty/openal/utils/SCUT_KEMAR.def
	rm -rf 3rdparty/openal/utils/alsoft-config/CMakeLists.txt
	rm -rf 3rdparty/openal/utils/alsoft-config/main.cpp
	rm -rf 3rdparty/openal/utils/alsoft-config/mainwindow.cpp
	rm -rf 3rdparty/openal/utils/alsoft-config/mainwindow.h
	rm -rf 3rdparty/openal/utils/alsoft-config/mainwindow.ui
	rm -rf 3rdparty/openal/utils/alsoft-config/verstr.cpp
	rm -rf 3rdparty/openal/utils/alsoft-config/verstr.h
	rm -rf 3rdparty/openal/utils/getopt.c
	rm -rf 3rdparty/openal/utils/getopt.h
	rm -rf 3rdparty/openal/utils/makemhr/loaddef.cpp
	rm -rf 3rdparty/openal/utils/makemhr/loaddef.h
	rm -rf 3rdparty/openal/utils/makemhr/loadsofa.cpp
	rm -rf 3rdparty/openal/utils/makemhr/loadsofa.h
	rm -rf 3rdparty/openal/utils/makemhr/makemhr.cpp
	rm -rf 3rdparty/openal/utils/makemhr/makemhr.h
	rm -rf 3rdparty/openal/utils/openal-info.c
	rm -rf 3rdparty/openal/utils/sofa-info.cpp
	rm -rf 3rdparty/openal/version.cmake
	rm -rf 3rdparty/openal/version.h.in

	# Restore generated sources.
	cp /tmp/config.h 3rdparty/openal/config.h
	cp /tmp/version.h 3rdparty/openal/version.h
	cp /tmp/bsinc_inc.h 3rdparty/openal/bsinc_inc.h
	echo ""
	echo "!!! Remember to update ALSOFT_VERSION !!!"

	# Add changes and commit.
	git add -f 3rdparty/openal
	git commit -m "3rdparty: update openal"
}

update_dear_imgui () {
	# Download latest dear-imgui.
	rm -rf 3rdparty/ocornut-imgui
	git clone https://github.com/ocornut/imgui 3rdparty/ocornut-imgui --branch=docking --depth=1

	# Cleanup.
	rm -rf 3rdparty/ocornut-imgui/backends
	rm -rf 3rdparty/ocornut-imgui/docs
	rm -rf 3rdparty/ocornut-imgui/examples
	rm -rf 3rdparty/ocornut-imgui/misc
	rm -rf 3rdparty/ocornut-imgui/.editorconfig
	rm -rf 3rdparty/ocornut-imgui/.git
	rm -rf 3rdparty/ocornut-imgui/.gitattributes
	rm -rf 3rdparty/ocornut-imgui/.github
	rm -rf 3rdparty/ocornut-imgui/.gitignore

	# Add changes and commit.
	git add -f 3rdparty/ocornut-imgui
	git commit -m "3rdparty: update ocornut-imgui"
}

update_nativefiledialog () {
	# Download latest nativefiledialog.
	rm -rf 3rdparty/nativefiledialog
	git clone https://github.com/mlabbe/nativefiledialog 3rdparty/nativefiledialog --depth=1

	# Cleanup.
	rm -r 3rdparty/nativefiledialog/build
	rm -r 3rdparty/nativefiledialog/docs
	rm -r 3rdparty/nativefiledialog/screens
	rm -r 3rdparty/nativefiledialog/test
	rm 3rdparty/nativefiledialog/.gitignore
	rm 3rdparty/nativefiledialog/README.md
	rm -rf 3rdparty/nativefiledialog/.git
	rm -rf 3rdparty/nativefiledialog/.github

	# Add changes and commit.
	git add -f 3rdparty/nativefiledialog
	git commit -m "3rdparty: update nativefiledialog"
}

update_sphinx_rtd_theme () {
	# Download latest sphinx_rtd_theme.
	RTD_THEME=$(mktemp -d)
	git clone https://github.com/readthedocs/sphinx_rtd_theme "${RTD_THEME}" --depth=1
	mv "${RTD_THEME}"/sphinx_rtd_theme docs/_themes/sphinx_rtd_theme
	rm -rf "${RTD_THEME}"

	# Add changes and commit.
	git add -f docs/_themes/sphinx_rtd_theme
	git commit -m "docs: update sphinx_rtd_theme"
}

print_help () {
	echo "Usage: scripts/upgrade.sh <dependency-name>"
}

while true; do
	case "$1" in
	luajit)
		update_luajit
		exit $?
		;;
	bgfx)
		update_bgfx
		exit $?
		;;
	bullet)
		update_bullet
		exit $?
		;;
	openal)
		update_openal
		exit $?
		;;
	dear_imgui)
		update_dear_imgui
		exit $?
		;;
	nativefiledialog)
		update_nativefiledialog
		exit $?
		;;
	sphinx_rtd_theme)
		update_sphinx_rtd_theme
		exit $?
		;;
	*)
		echo "Unknown option or target \`$1\`"
		print_help
		exit 1
		;;
	esac
done
