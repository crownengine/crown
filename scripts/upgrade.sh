#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

set -eu

git_clone () {
	local DEST=$1
	local REPO=$2
	local BRANCH=$3
	local COMMIT=${4-}
	local DEPTH=1

	if [ ! -z "${COMMIT}" ]; then
		DEPTH=250 # Ensure enough commits are available for COMMIT option to work.
	fi

	git clone "${REPO}" "${DEST}" --branch="${BRANCH}" --depth="${DEPTH}" --separate-git-dir="$(mktemp -u)"

	if [ ! -z "${COMMIT}" ]; then
		pushd "${PWD}" > /dev/null
		cd "${DEST}"                        \
			&& git reset --hard "${COMMIT}" \
			&& rm .git
		popd > /dev/null
	fi
}

update_luajit () {
	local DEST=3rdparty/luajit
	local REPO=https://github.com/LuaJIT/LuaJIT.git
	local BRANCH=v2.1

	# Download latest luajit.
	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup.
	rm -rf "${DEST}"/.gitignore
	rm -rf "${DEST}"/doc
	rm -rf "${DEST:?}"/etc
	rm -rf "${DEST}"/Makefile
	rm -rf "${DEST}"/README

	# Bump affected resources versions.
	RESOURCE_TYPES_H=src/resource/types.h

	sed -Ei 's/(.*RESOURCE_VERSION_SCRIPT)(.*RESOURCE_VERSION\()([0-9]+)(.*)/echo "\1\2$((\3 + 1))\4"/ge' src/resource/types.h

	# Add changes and commit.
	git add -f "${DEST}"
	git add "${RESOURCE_TYPES_H}"
	git commit -m "3rdparty: update luajit"
}

update_bx () {
	local DEST=3rdparty/bx
	local REPO=https://github.com/bkaradzic/bx
	local BRANCH=master

	# Download latest bx.
	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup bx.
	rm -rf "${DEST}"/.appveyor.yml
	rm -rf "${DEST}"/.editorconfig
	rm -rf "${DEST}"/.gitattributes
	rm -rf "${DEST}"/.gitignore
	rm -rf "${DEST}"/.travis.yml
	rm -rf "${DEST}"/3rdparty/catch
	rm -rf "${DEST}"/makefile
	rm -rf "${DEST}"/README.md
	rm -rf "${DEST}"/scripts/bin2c.lua
	rm -rf "${DEST}"/scripts/lemon.lua
	rm -rf "${DEST}"/tests
	rm -rf "${DEST}"/tools

	# Ensure BX_CONFIG_DEBUG is set in development builds.
	sed -i 's/{ "Release" }$/{ "Release or development" }/' "${DEST}"/scripts/bx.lua
	sed -i 's/{ "Release" }$/{ "Release or development" }/' "${DEST}"/scripts/bx.lua

	git add -f "${DEST}"
}

update_bimg () {
	local DEST=3rdparty/bimg
	local REPO=https://github.com/bkaradzic/bimg
	local BRANCH=master

	# Download latest bimg.
	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup bimg.
	rm -rf "${DEST}"/.appveyor.yml
	rm -rf "${DEST}"/.editorconfig
	rm -rf "${DEST}"/.gitattributes
	rm -rf "${DEST}"/.gitignore
	rm -rf "${DEST}"/.travis.yml
	rm -rf "${DEST}"/makefile
	rm -rf "${DEST}"/README.md

	git add -f "${DEST}"
}

update_bgfx () {
	local DEST=3rdparty/bgfx
	local REPO=https://github.com/bkaradzic/bgfx
	local BRANCH=master

	# Update bgfx's dependencies.
	update_bx
	update_bimg

	# Download latest bgfx.
	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Regenerate samples/core/shaders/common.shader.
	COMMON_SHADER=samples/core/shaders/common.shader

	rm "${COMMON_SHADER}"
	{
		echo "render_states = {"
		echo "	default = {"
		echo "		states = {"
		echo "			rgb_write_enable = true"
		echo "			alpha_write_enable = true"
		echo "			depth_write_enable = true"
		echo "			depth_enable = true"
		echo "			blend_enable = false"
		echo "			depth_func = \"lequal\""
		echo "			blend_src = \"src_alpha\""
		echo "			blend_dst = \"inv_src_alpha\""
		echo "			blend_equation = \"add\""
		echo "			cull_mode = \"cw\""
		echo "			primitive_type = \"pt_triangles\""
		echo "		}"
		echo "	}"
		echo ""
		echo "	opacity = {"
		echo "		inherit = \"default\""
		echo "		states = {"
		echo "			cull_mode = \"none\""
		echo "			depth_write_enable = false"
		echo "			depth_enable = false"
		echo "			blend_enable = true"
		echo "			blend_src = \"src_alpha\""
		echo "			blend_dst = \"inv_src_alpha\""
		echo "			\"defined(DEPTH_ENABLED)\" = {"
		echo "				depth_enable = true"
		echo "			}"
		echo "		}"
		echo "	}"
		echo ""
		echo "}"
		echo ""
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
		sed 's/^/\t\t\t/' "${DEST}"/src/bgfx_shader.sh "${DEST}"/examples/common/shaderlib.sh
		echo "		\"\"\""
		echo "	}"
		echo "}"
	} >> "${COMMON_SHADER}"

	# Remove trailing tabs.
	sed -i 's/\t*$//' "${COMMON_SHADER}"

	# Cleanup bgfx.
	rm -rf "${DEST}"/.appveyor.yml
	rm -rf "${DEST}"/.editorconfig
	rm -rf "${DEST}"/.gitattributes
	rm -rf "${DEST}"/.github
	rm -rf "${DEST}"/.gitignore
	rm -rf "${DEST}"/.travis.yml
	rm -rf "${DEST}"/3rdparty/.editorconfig
	rm -rf "${DEST}"/3rdparty/cgltf
	rm -rf "${DEST}"/3rdparty/dear-imgui
	rm -rf "${DEST}"/3rdparty/freetype
	rm -rf "${DEST}"/3rdparty/glslang/build
	rm -rf "${DEST}"/3rdparty/glslang/build_info.py
	rm -rf "${DEST}"/3rdparty/glslang/license-checker.cfg
	rm -rf "${DEST}"/3rdparty/iconfontheaders
	rm -rf "${DEST}"/3rdparty/meshoptimizer
	rm -rf "${DEST}"/3rdparty/sdf
	rm -rf "${DEST}"/3rdparty/stb
	rm -rf "${DEST}"/bindings
	rm -rf "${DEST}"/CODEOWNERS
	rm -rf "${DEST}"/CONTRIBUTING.md
	rm -rf "${DEST}"/examples
	rm -rf "${DEST}"/makefile
	rm -rf "${DEST}"/README.md
	rm -rf "${DEST}"/scripts/bgfx-codegen.lua
	rm -rf "${DEST}"/scripts/bgfx.doxygen
	rm -rf "${DEST}"/scripts/bgfx.idl
	rm -rf "${DEST}"/scripts/bindings-*.lua
	rm -rf "${DEST}"/scripts/build.ninja
	rm -rf "${DEST}"/scripts/codegen.lua
	rm -rf "${DEST}"/scripts/doxygen.lua
	rm -rf "${DEST}"/scripts/geometryc.lua
	rm -rf "${DEST}"/scripts/geometryv.lua
	rm -rf "${DEST}"/scripts/idl.lua
	rm -rf "${DEST}"/scripts/shader-embeded.mk
	rm -rf "${DEST}"/scripts/shader.mk
	rm -rf "${DEST}"/scripts/temp.bgfx.h
	rm -rf "${DEST}"/scripts/temp.bgfx.idl.inl
	rm -rf "${DEST}"/scripts/temp.defines.h
	rm -rf "${DEST}"/scripts/texturev.lua
	rm -rf "${DEST}"/scripts/tools.mk
	rm -rf "${DEST}"/src/bgfx_compute.sh
	rm -rf "${DEST}"/src/bgfx_shader.sh
	rm -rf "${DEST}"/src/fs_clear0.sc
	rm -rf "${DEST}"/src/fs_clear1.sc
	rm -rf "${DEST}"/src/fs_clear2.sc
	rm -rf "${DEST}"/src/fs_clear3.sc
	rm -rf "${DEST}"/src/fs_clear4.sc
	rm -rf "${DEST}"/src/fs_clear5.sc
	rm -rf "${DEST}"/src/fs_clear6.sc
	rm -rf "${DEST}"/src/fs_clear7.sc
	rm -rf "${DEST}"/src/fs_debugfont.sc
	rm -rf "${DEST}"/src/makefile
	rm -rf "${DEST}"/src/varying.def.sc
	rm -rf "${DEST}"/src/vs_clear.sc
	rm -rf "${DEST}"/src/vs_debugfont.sc
	rm -rf "${DEST}"/tools/geometryc
	rm -rf "${DEST}"/tools/geometryv
	rm -rf "${DEST}"/tools/texturev
	rm -rf "${DEST}"/tools/bin
	rm -rf "${DEST}"/docs
	sed -i '/dofile \"texturev.lua\"/d' "${DEST}"/scripts/genie.lua
	sed -i '/dofile \"geometryc.lua\"/d' "${DEST}"/scripts/genie.lua
	sed -i '/dofile \"geometryv.lua\"/d' "${DEST}"/scripts/genie.lua

	# Bump affected resources versions.
	RESOURCE_TYPES_H=src/resource/types.h

	sed -Ei 's/(.*RESOURCE_VERSION_TEXTURE)(.*RESOURCE_VERSION\()([0-9]+)(.*)/echo "\1\2$((\3 + 1))\4"/ge' src/resource/types.h
	sed -Ei 's/(.*RESOURCE_VERSION_SHADER)(.*RESOURCE_VERSION\()([0-9]+)(.*)/echo "\1\2$((\3 + 1))\4"/ge' src/resource/types.h

	# Add changes and commit.
	git add -f "${DEST}"
	git add "${COMMON_SHADER}"
	git add "${RESOURCE_TYPES_H}"
	git commit -m "3rdparty: update bx, bimg and bgfx"
}

update_bullet () {
	local DEST=3rdparty/bullet3
	local REPO=https://github.com/bulletphysics/bullet3
	local BRANCH=master

	# Download latest bullet3.
	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup.
	rm -rf "${DEST}"/.ci
	rm -rf "${DEST}"/.github
	rm -rf "${DEST}"/.gitignore
	rm -rf "${DEST}"/.style.yapf
	rm -rf "${DEST}"/.travis.yml
	rm -rf "${DEST}"/_clang-format
	rm -rf "${DEST}"/appveyor.yml
	rm -rf "${DEST}"/AUTHORS.txt
	rm -rf "${DEST}"/build3
	rm -rf "${DEST}"/build_cmake_pybullet_double.sh
	rm -rf "${DEST}"/build_visual_studio_vr_pybullet_double.bat
	rm -rf "${DEST}"/build_visual_studio_vr_pybullet_double_cmake.bat
	rm -rf "${DEST}"/build_visual_studio_vr_pybullet_double_dynamic.bat
	rm -rf "${DEST}"/build_visual_studio_without_pybullet_vr.bat
	rm -rf "${DEST}"/bullet.pc.cmake
	rm -rf "${DEST}"/BulletConfig.cmake.in
	rm -rf "${DEST}"/clang-format-all.sh
	rm -rf "${DEST}"/CMakeLists.txt
	rm -rf "${DEST}"/data
	rm -rf "${DEST}"/docs
	rm -rf "${DEST}"/Doxyfile
	rm -rf "${DEST}"/examples
	rm -rf "${DEST}"/Extras
	rm -rf "${DEST}"/MANIFEST.in
	rm -rf "${DEST}"/README.md
	rm -rf "${DEST}"/setup.py
	rm -rf "${DEST}"/src/*.cpp
	rm -rf "${DEST}"/src/*.h
	rm -rf "${DEST}"/src/.DS_Store
	rm -rf "${DEST}"/src/Bullet3Collision/
	rm -rf "${DEST}"/src/Bullet3Common
	rm -rf "${DEST}"/src/Bullet3Dynamics
	rm -rf "${DEST}"/src/Bullet3Geometry
	rm -rf "${DEST}"/src/Bullet3OpenCL
	rm -rf "${DEST}"/src/Bullet3Serialize
	rm -rf "${DEST}"/src/BulletCollision/CollisionDispatch/btCollisionWorldImporter.cpp
	rm -rf "${DEST}"/src/BulletCollision/Gimpact
	rm -rf "${DEST}"/src/BulletInverseDynamics
	rm -rf "${DEST}"/src/clew
	rm -rf "${DEST}"/test
	rm -rf "${DEST}"/UseBullet.cmake
	rm -rf "${DEST}"/VERSION
	rm -rf "${DEST}"/xcode.command
	find "${DEST}" -type f -name 'CMakeLists.txt' -exec rm {} +
	find "${DEST}" -type f -name 'premake4.lua' -exec rm {} +

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "3rdparty: update bullet3"
}

update_openal () {
	local DEST=3rdparty/openal
	local REPO=https://github.com/kcat/openal-soft
	local BRANCH=master

	# Backup generated headers.
	cp "${DEST}"/config.h /tmp/config.h
	cp "${DEST}"/version.h /tmp/version.h
	cp "${DEST}"/bsinc_inc.h /tmp/bsinc_inc.h

	# Download latest sources.
	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup.
	rm -rf "${DEST}"/.gitignore
	rm -rf "${DEST}"/.travis.yml
	rm -rf "${DEST}"/CMakeLists.txt
	rm -rf "${DEST}"/README.md
	rm -rf "${DEST}"/XCompile-Android.txt
	rm -rf "${DEST}"/XCompile.txt
	rm -rf "${DEST}"/alsoftrc.sample
	rm -rf "${DEST}"/appveyor.yml
	rm -rf "${DEST}"/build/.empty
	rm -rf "${DEST}"/cmake/FindALSA.cmake
	rm -rf "${DEST}"/cmake/FindAudioIO.cmake
	rm -rf "${DEST}"/cmake/FindDSound.cmake
	rm -rf "${DEST}"/cmake/FindFFmpeg.cmake
	rm -rf "${DEST}"/cmake/FindJACK.cmake
	rm -rf "${DEST}"/cmake/FindMySOFA.cmake
	rm -rf "${DEST}"/cmake/FindOSS.cmake
	rm -rf "${DEST}"/cmake/FindOpenSL.cmake
	rm -rf "${DEST}"/cmake/FindPortAudio.cmake
	rm -rf "${DEST}"/cmake/FindPulseAudio.cmake
	rm -rf "${DEST}"/cmake/FindQSA.cmake
	rm -rf "${DEST}"/cmake/FindSDL2.cmake
	rm -rf "${DEST}"/cmake/FindSDL_sound.cmake
	rm -rf "${DEST}"/cmake/FindSoundIO.cmake
	rm -rf "${DEST}"/cmake/FindWindowsSDK.cmake
	rm -rf "${DEST}"/config.h.in
	rm -rf "${DEST}"/docs/3D7.1.txt
	rm -rf "${DEST}"/docs/ambdec.txt
	rm -rf "${DEST}"/docs/ambisonics.txt
	rm -rf "${DEST}"/docs/env-vars.txt
	rm -rf "${DEST}"/docs/hrtf.txt
	rm -rf "${DEST}"/examples/alffplay.cpp
	rm -rf "${DEST}"/examples/alhrtf.c
	rm -rf "${DEST}"/examples/allatency.c
	rm -rf "${DEST}"/examples/alloopback.c
	rm -rf "${DEST}"/examples/almultireverb.c
	rm -rf "${DEST}"/examples/alplay.c
	rm -rf "${DEST}"/examples/alrecord.c
	rm -rf "${DEST}"/examples/alreverb.c
	rm -rf "${DEST}"/examples/alstream.c
	rm -rf "${DEST}"/examples/altonegen.c
	rm -rf "${DEST}"/examples/common/alhelpers.c
	rm -rf "${DEST}"/examples/common/alhelpers.h
	rm -rf "${DEST}"/native-tools/CMakeLists.txt
	rm -rf "${DEST}"/native-tools/bin2h.c
	rm -rf "${DEST}"/native-tools/bsincgen.c
	rm -rf "${DEST}"/openal.pc.in
	rm -rf "${DEST}"/resources/openal32.rc
	rm -rf "${DEST}"/resources/resource.h
	rm -rf "${DEST}"/resources/router.rc
	rm -rf "${DEST}"/resources/soft_oal.rc
	rm -rf "${DEST}"/router/al.cpp
	rm -rf "${DEST}"/router/alc.cpp
	rm -rf "${DEST}"/router/router.cpp
	rm -rf "${DEST}"/router/router.h
	rm -rf "${DEST}"/utils/CIAIR.def
	rm -rf "${DEST}"/utils/IRC_1005.def
	rm -rf "${DEST}"/utils/MIT_KEMAR.def
	rm -rf "${DEST}"/utils/MIT_KEMAR_sofa.def
	rm -rf "${DEST}"/utils/SCUT_KEMAR.def
	rm -rf "${DEST}"/utils/alsoft-config/CMakeLists.txt
	rm -rf "${DEST}"/utils/alsoft-config/main.cpp
	rm -rf "${DEST}"/utils/alsoft-config/mainwindow.cpp
	rm -rf "${DEST}"/utils/alsoft-config/mainwindow.h
	rm -rf "${DEST}"/utils/alsoft-config/mainwindow.ui
	rm -rf "${DEST}"/utils/alsoft-config/verstr.cpp
	rm -rf "${DEST}"/utils/alsoft-config/verstr.h
	rm -rf "${DEST}"/utils/getopt.c
	rm -rf "${DEST}"/utils/getopt.h
	rm -rf "${DEST}"/utils/makemhr/loaddef.cpp
	rm -rf "${DEST}"/utils/makemhr/loaddef.h
	rm -rf "${DEST}"/utils/makemhr/loadsofa.cpp
	rm -rf "${DEST}"/utils/makemhr/loadsofa.h
	rm -rf "${DEST}"/utils/makemhr/makemhr.cpp
	rm -rf "${DEST}"/utils/makemhr/makemhr.h
	rm -rf "${DEST}"/utils/openal-info.c
	rm -rf "${DEST}"/utils/sofa-info.cpp
	rm -rf "${DEST}"/version.cmake
	rm -rf "${DEST}"/version.h.in
	rm -rf "${DEST}"/.github

	# Restore generated sources.
	cp /tmp/config.h "${DEST}"/config.h
	cp /tmp/version.h "${DEST}"/version.h
	cp /tmp/bsinc_inc.h "${DEST}"/bsinc_inc.h

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "3rdparty: update openal"

	echo ""
	echo "!!! Remember to update ALSOFT_VERSION !!!"
}

update_sphinx_rtd_theme () {
	local REPO=https://github.com/readthedocs/sphinx_rtd_theme
	local DEST=docs/_themes/sphinx_rtd_theme
	local BRANCH=master

	local RTD_THEME=$(mktemp -d)

	# Download latest sphinx_rtd_theme.
	git_clone "${RTD_THEME}" "${REPO}" "${BRANCH}"

	rm -rf "${DEST}"
	mv "${RTD_THEME}"/sphinx_rtd_theme "${DEST}"

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "docs: update sphinx_rtd_theme"
}

update_sphinx_sitemap () {
	local REPO=https://github.com/jdillard/sphinx-sitemap
	local DEST=docs/_extensions/sphinx_sitemap
	local BRANCH=master

	local SITEMAP=$(mktemp -d)

	# Download latest sphinx_rtd_theme.
	git_clone "${SITEMAP}" "${REPO}" "${BRANCH}"

	rm -rf "${DEST}"
	mv "${SITEMAP}"/sphinx_sitemap "${DEST}"

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "docs: update sphinx-sitemap"
}

update_sphinx_lubg () {
	local REPO=https://github.com/mgeier/sphinx-last-updated-by-git/
	local DEST=docs/_extensions/sphinx_last_updated_by_git.py
	local BRANCH=master

	local LUBG=$(mktemp -d)

	# Download latest sphinx_rtd_theme.
	git_clone "${LUBG}" "${REPO}" "${BRANCH}"

	rm -rf "${DEST}"
	mv "${LUBG}"/src/sphinx_last_updated_by_git.py "${DEST}"

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "docs: update sphinx-last-updated-by-git"
}

update_sphinx_opengraph () {
	local REPO=https://github.com/sphinx-doc/sphinxext-opengraph
	local DEST=docs/_extensions/sphinxext/opengraph
	local BRANCH=master

	local OPENGRAPH=$(mktemp -d)

	# Download latest sphinx_rtd_theme.
	git_clone "${OPENGRAPH}" "${REPO}" "${BRANCH}"

	rm -rf "${DEST}"
	mv "${OPENGRAPH}"/sphinxext/opengraph "${DEST}"

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "docs: update sphinxext-opengraph"
}

update_gtk_theme () {
	local REPO=https://gitlab.gnome.org/GNOME/gtk.git
	local DEST=tools/level_editor/resources/theme/Adwaita
	local BRANCH=gtk-3-24

	local GTK_THEME=$(mktemp -d)

	# Download latest sphinx_rtd_theme.
	git_clone "${GTK_THEME}" "${REPO}" "${BRANCH}"

	rm -rf "${DEST}"
	mv "${GTK_THEME}"/gtk/theme/Adwaita "${DEST}"

	sed -i 's/resource:\/\/\/org\/gtk\/libgtk\/theme\/Adwaita\/gtk-contained.css/resource:\/\/\/org\/crown\/level_editor\/theme\/Adwaita\/gtk-contained.css/' "${DEST}"/gtk.css
	sed -i 's/resource:\/\/\/org\/gtk\/libgtk\/theme\/Adwaita\/gtk-contained-dark.css/resource:\/\/\/org\/crown\/level_editor\/theme\/Adwaita\/gtk-contained-dark.css/' "${DEST}"/gtk-dark.css

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "tools: update GTK theme"
}

update_tinyexpr () {
	# Download latest tinyexpr.
	local REPO=https://github.com/codeplea/tinyexpr
	local DEST=3rdparty/tinyexpr
	local BRANCH=master

	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup.
	rm -r "${DEST}"/doc
	rm "${DEST}"/benchmark.c
	rm "${DEST}"/CONTRIBUTING
	rm "${DEST}"/example2.c
	rm "${DEST}"/example3.c
	rm "${DEST}"/example.c
	rm "${DEST}"/Makefile
	rm "${DEST}"/minctest.h
	rm "${DEST}"/README.md
	rm "${DEST}"/repl.c
	rm "${DEST}"/smoke.c
	rm "${DEST}"/.travis.yml

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "3rdparty: update tinyexpr"
}

update_ufbx () {
	# Download latest tinyexpr.
	local REPO=https://github.com/ufbx/ufbx
	local DEST=3rdparty/ufbx
	local BRANCH=v0.20.0

	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup.
	rm -r "${DEST}"/.git
	rm -r "${DEST}"/.github
	rm -r "${DEST}"/bindgen
	rm -r "${DEST}"/data
	rm -r "${DEST}"/examples
	rm -r "${DEST}"/extra
	rm -r "${DEST}"/misc
	rm -r "${DEST}"/test
	rm "${DEST}"/.gitattributes
	rm "${DEST}"/.gitignore
	rm "${DEST}"/README.md

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "3rdparty: update ufbx"
}

update_stb () {
	# Download latest stb libs.
	local REPO=https://github.com/nothings/stb
	local DEST=3rdparty/stb
	local BRANCH=master

	rm -rf "${DEST}"
	git_clone "${DEST}" "${REPO}" "${BRANCH}"

	# Cleanup.
	rm -r "${DEST}"/.git
	rm -r "${DEST}"/.github
	rm -r "${DEST}"/data
	rm -r "${DEST}"/deprecated
	rm -r "${DEST}"/docs
	rm -r "${DEST}"/stb_image_resize_test
	rm -r "${DEST}"/tests
	rm -r "${DEST}"/tools
	rm "${DEST}"/.gitignore
	rm "${DEST}"/.travis.yml
	rm "${DEST}"/.gitignore
	rm "${DEST}"/README.md
	rm "${DEST}"/stb_c_lexer.h
	rm "${DEST}"/stb_connected_components.h
	rm "${DEST}"/stb_divide.h
	rm "${DEST}"/stb_ds.h
	rm "${DEST}"/stb_dxt.h
	rm "${DEST}"/stb_easy_font.h
	rm "${DEST}"/stb_herringbone_wang_tile.h
	rm "${DEST}"/stb_hexwave.h
	rm "${DEST}"/stb_image.h
	rm "${DEST}"/stb_image_resize2.h
	rm "${DEST}"/stb_image_write.h
	rm "${DEST}"/stb_include.h
	rm "${DEST}"/stb_leakcheck.h
	rm "${DEST}"/stb_perlin.h
	rm "${DEST}"/stb_textedit.h
	rm "${DEST}"/stb_tilemap_editor.h
	rm "${DEST}"/stb_voxel_render.h

	# Add changes and commit.
	git add -f "${DEST}"
	git commit -m "3rdparty: update stb"
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
	sphinx_rtd_theme)
		update_sphinx_rtd_theme
		exit $?
		;;
	sphinx_sitemap)
		update_sphinx_sitemap
		exit $?
		;;
	sphinx_lubg)
		update_sphinx_lubg
		exit $?
		;;
	sphinx_opengraph)
		update_sphinx_opengraph
		exit $?
		;;
	tinyexpr)
		update_tinyexpr
		exit $?
		;;
	gtk_theme)
		update_gtk_theme
		exit $?
		;;
	ufbx)
		update_ufbx
		exit $?
		;;
	stb)
		update_stb
		exit $?
		;;
	*)
		echo "Unknown option or target \`$1\`"
		print_help
		exit 1
		;;
	esac
done
