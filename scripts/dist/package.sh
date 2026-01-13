#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

set -eu

. scripts/dist/version.sh

NOCONFIRM=0
ARGS=()

while [ $# -gt 0 ]; do
	case "$1" in
	-h|--help)
		echo "Usage: $0 [options] <platform> <arch> [master]"
		echo ""
		echo "Options:"
		echo "  --noconfirm  Skip any user confirmations."
		echo ""
		echo "Examples:"
		echo "$0 android arm64    # Create android-arm64 package. Version is inferred from config.h."
		echo "$0 linux x64 master # Create linux-x64 package. Append \"-master-<commit>\" to version name."
		echo ""
		exit 0
		;;
	--noconfirm)
		NOCONFIRM=1
		shift
		;;
	-*)
		echo "Unknown option $1"
		exit 1
		;;
	*)
		ARGS+=($1)
		shift
		;;
	esac
done

set -- "${ARGS[@]}"

PLATFORM=${1-}
ARCH=${2-}
MASTER=${3-}

# Validate platform/arch combination.
if [ "${PLATFORM}" = "android" ]; then
	if [ "${ARCH}" != "arm" ] && [ "${ARCH}" != "arm64" ]; then
		echo "Invalid architecture ${ARCH}"
		exit 1
	fi
elif [ "${PLATFORM}" = "html5" ]; then
	if [ "${ARCH}" != "wasm" ]; then
		echo "Invalid architecture ${ARCH}"
		exit 1
	fi
elif [ "${PLATFORM}" = "linux" ]; then
	if [ "${ARCH}" != "x64" ]; then
		echo "Invalid architecture ${ARCH}"
		exit 1
	fi
elif [ "${PLATFORM}" = "windows" ]; then
	if [ "${ARCH}" != "x64" ]; then
		echo "Invalid architecture ${ARCH}"
		exit 1
	fi
else
	echo "Invalid platform ${PLATFORM}"
	exit 1
fi

VERSION=$(crown_version)
BUILD_JOBS=$(nproc)

if [ "${PLATFORM}" = "windows" ]; then
	EXE_SUFFIX=.exe
else
	EXE_SUFFIX=
fi

if [ "${MASTER}" = "master" ]; then
	VERSION_SUFFIX="-master-"$(git rev-parse --short HEAD)
else
	VERSION_SUFFIX=
fi

# Destination folder.
VERSIONNAME=crown-"${VERSION}""${VERSION_SUFFIX}"
PACKAGENAME="${VERSIONNAME}"-${PLATFORM}-${ARCH}

# Tarball name.
TAR="tar -cf"
TARBALLEXT=tar
TARBALLNAME="${PACKAGENAME}.${TARBALLEXT}"

if [ "${NOCONFIRM}" -eq 0 ]; then
	echo "Crown '${VERSION}' will be packaged as '${TARBALLNAME}'"
	echo "Continue? [y/N]"
	read -r answer
	if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
		echo "Bye."
		exit;
	fi
fi

# Cleanup previous builds.
make clean
rm -rf "${TARBALLNAME}"
rm -rf "${PACKAGENAME}"

# Build engine and tools.
if [ "${PLATFORM}" = "android" ]; then
	make android-"${ARCH}"-development MAKE_JOBS="${BUILD_JOBS}"
	make android-"${ARCH}"-release MAKE_JOBS="${BUILD_JOBS}"
elif [ "${PLATFORM}" = "html5" ]; then
	make wasm-development MAKE_JOBS="${BUILD_JOBS}"
	make wasm-release MAKE_JOBS="${BUILD_JOBS}"
elif [ "${PLATFORM}" = "linux" ]; then
	make tools-linux-release32 MAKE_JOBS="${BUILD_JOBS}"
	make tools-linux-release64 MAKE_JOBS="${BUILD_JOBS}"
	make crown-launcher-linux-release64 MAKE_JOBS="${BUILD_JOBS}"
	make linux-release64 MAKE_JOBS="${BUILD_JOBS}"
elif [ "${PLATFORM}" = "windows" ]; then
	# Build 64bit tools first.
	export MINGW=/mingw64
	export PATH="${MINGW}/bin:${PATH}"
	make tools-mingw-release64 MAKE_JOBS="${BUILD_JOBS}"
	make crown-launcher-mingw-release64 MAKE_JOBS="${BUILD_JOBS}"
	make mingw-release64 MAKE_JOBS="${BUILD_JOBS}"

	# Copy required DLLs.
	ldd build/mingw64/bin/crown-editor-release.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" build/mingw64/bin

	# Copy GTK-related executables.
	cp /mingw64/bin/fc-cache.exe                    build/mingw64/bin
	cp /mingw64/bin/fc-cat.exe                      build/mingw64/bin
	cp /mingw64/bin/fc-list.exe                     build/mingw64/bin
	cp /mingw64/bin/fc-match.exe                    build/mingw64/bin
	cp /mingw64/bin/fc-pattern.exe                  build/mingw64/bin
	cp /mingw64/bin/fc-query.exe                    build/mingw64/bin
	cp /mingw64/bin/fc-scan.exe                     build/mingw64/bin
	cp /mingw64/bin/fc-validate.exe                 build/mingw64/bin
	cp /mingw64/bin/gdbus.exe                       build/mingw64/bin
	cp /mingw64/bin/gdk-pixbuf-query-loaders.exe    build/mingw64/bin
	cp /mingw64/bin/gspawn-win64-helper-console.exe build/mingw64/bin
	cp /mingw64/bin/gspawn-win64-helper.exe         build/mingw64/bin
	cp /mingw64/bin/gtk-query-immodules-3.0.exe     build/mingw64/bin
	cp /mingw64/bin/gtk-update-icon-cache.exe       build/mingw64/bin

	# Copy additional DLLs.
	cp /mingw64/bin/liblzma-5.dll   build/mingw64/bin
	cp /mingw64/bin/librsvg-2-2.dll build/mingw64/bin
	cp /mingw64/bin/libxml2-16.dll  build/mingw64/bin

	# Copy GDK pixbuf loaders.
	mkdir -p build/mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders
	cp -r /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/*pixbufloader*.dll build/mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders
	cp -r /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache              build/mingw64/lib/gdk-pixbuf-2.0/2.10.0

	# Copy GLib schemas.
	mkdir -p build/mingw64/share/glib-2.0/schemas
	cp -r /mingw64/share/glib-2.0/schemas/gschemas.compiled build/mingw64/share/glib-2.0/schemas

	# Copy Adwaita and hicolor icons.
	mkdir -p build/mingw64/share/icons/Adwaita/scalable
	mkdir -p build/mingw64/share/icons/Adwaita/symbolic
	mkdir -p build/mingw64/share/icons/Adwaita/cursors
	cp -r /mingw64/share/icons/Adwaita/icon-theme.cache   build/mingw64/share/icons/Adwaita
	cp -r /mingw64/share/icons/Adwaita/index.theme        build/mingw64/share/icons/Adwaita
	cp -r /mingw64/share/icons/Adwaita/scalable/*         build/mingw64/share/icons/Adwaita/scalable
	cp -r /mingw64/share/icons/Adwaita/symbolic/*         build/mingw64/share/icons/Adwaita/symbolic
	cp -r /mingw64/share/icons/Adwaita/cursors/*          build/mingw64/share/icons/Adwaita/cursors
	cp -r /mingw64/share/icons/hicolor                    build/mingw64/share/icons

	# Switch to 32bit toolchain, re-generate projects and build 32bit tools.
	export MINGW=/mingw32
	export PATH="${MINGW}/bin:${PATH}"
	make -B tools-mingw-release32 MAKE_JOBS="${BUILD_JOBS}"

	# Rename mingw* to windows*.
	mv build/mingw64 build/windows64
	mv build/mingw32 build/windows32
fi

# Strip unnecessary files from build dir.
find build -iname 'obj'               \
	-o -iname     'projects'          \
	-o -wholename 'android-*/bin/jit' \
	-o -iname     '*.a'               \
	-o -iname     '*.exp'             \
	-o -iname     '*.ilk'             \
	-o -iname     '*.lib'             \
	-o -iname     'bgfx-*'            \
	-o -iname     'bimg_decode-*'     \
	-o -iname     'bimg_encode-*'     \
	-o -iname     'bimg-*'            \
	-o -iname     'bullet-*'          \
	-o -iname     'bx-*'              \
	| tr '\n' '\0'                    \
	| xargs -0 -n1 rm -rf

# Create release package from build dir.
PARTIALSDIR=dist/"${VERSIONNAME}"/partials
PARTIALPACKAGE="${PARTIALSDIR}"/"${PACKAGENAME}"
mkdir -p "${PARTIALPACKAGE}"
mv build "${PARTIALPACKAGE}"/platforms

if [ "${PLATFORM}" = "linux" ] || [ "${PLATFORM}" = "windows" ]; then
	# Copy exporters, samples etc. to package dir.
	cp    LICENSE   "${PARTIALPACKAGE}"
	cp -r exporters "${PARTIALPACKAGE}"
	cp -r samples   "${PARTIALPACKAGE}"
	mv    "${PARTIALPACKAGE}"/samples/core "${PARTIALPACKAGE}"

	if [ "${PLATFORM}" = "linux" ]; then
		# Copy crown-launcher.
		mv "${PARTIALPACKAGE}"/platforms/linux64/bin/crown-launcher-release "${PARTIALPACKAGE}"/crown-launcher

		# Copy app icon and .desktop file.
		cp tools/level_editor/resources/org.crownengine.Crown.desktop "${PARTIALPACKAGE}"
		cp tools/level_editor/resources/icons/hicolor/scalable/apps/org.crownengine.Crown.svg "${PARTIALPACKAGE}"
	elif [ "${PLATFORM}" = "windows" ]; then
		# Copy crown-launcher.
		mv "${PARTIALPACKAGE}"/platforms/windows64/bin/crown-launcher-release.exe "${PARTIALPACKAGE}"/crown-launcher.exe
	fi
fi
