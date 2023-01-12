#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2023 Daniele Bartolini et al.

if [ $# -lt 1 ]; then
	echo "Usage: $0 <platform> <arch> [version]"
	echo ""
	echo "e.g."
	echo "$0 linux x64 0.38.0" # Create package for linux x64, v0.38.0
	echo "$0 android arm64"    # Version is inferred from last Git tag
	exit
fi

PLATFORM=$1
ARCH=$2
VERSION=$3

# Validate platform/arch combination.
if [ "${PLATFORM}" = "android" ]; then
	if [ "${ARCH}" != "arm" ] && [ "${ARCH}" != "arm64" ]; then
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

# If version is not specified, extract it from most recent tag name.
if [ -z "${VERSION}" ]; then
	VERSION=$(git tag | tail -n 1| cut -c2-)
fi
BUILD_JOBS=$(nproc)

if [ "${PLATFORM}" = "windows" ]; then
	EXE_SUFFIX=.exe
else
	EXE_SUFFIX=
fi

# Build folder.
BINARIES_DIR="${PLATFORM}"64
if [ "${PLATFORM}" = "android" ]; then
	BINARIES_DIR="${PLATFORM}-${ARCH}"
fi

# Destination folder.
PACKAGENAME=crown-"${VERSION}"

# Tarball name.
ZIP="tar -zcf"
TARBALLEXTENSION=tar.gz
if [ "${PLATFORM}" = "windows" ]; then
	ZIP="zip -rq"
	TARBALLEXTENSION=zip
fi

TARBALLNAME="crown-${VERSION}-${PLATFORM}-${ARCH}.${TARBALLEXTENSION}"

echo "Crown v${VERSION} will be packaged as ${TARBALLNAME}"
echo "Continue? [y/N]"
read -r answer
if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
	echo "Bye."
	exit;
fi

# Cleanup previous builds.
rm -rf "${TARBALLNAME}"
rm -rf "${PACKAGENAME}"

# Switch to desired tag.
git checkout v"${VERSION}"

# Build engine and tools.
make clean
if [ "${PLATFORM}" = "android" ]; then
	export ANDROID_NDK_ROOT=~/android-sdk/ndk/21.0.6113669
	export ANDROID_NDK_ABI=23
	make android-"${ARCH}"-development MAKE_JOBS="${BUILD_JOBS}"
	make android-"${ARCH}"-release MAKE_JOBS="${BUILD_JOBS}"
elif [ "${PLATFORM}" = "linux" ]; then
	make tools-linux-release64 MAKE_JOBS="${BUILD_JOBS}"
	make linux-release64 MAKE_JOBS="${BUILD_JOBS}"
elif [ "${PLATFORM}" = "windows" ]; then
	/c/Windows/System32/cmd.exe //C "scripts\\dist\\windows-release.bat"
	export MINGW=/mingw64
	export PATH="${PATH}:${MINGW}/bin"
	make tools-mingw-release64 MAKE_JOBS="${BUILD_JOBS}"
fi

# Copy license to build dir.
cp LICENSE build

if [ "${PLATFORM}" = "linux" ] || [ "${PLATFORM}" = "windows" ]; then
	# Copy exporters, samples etc. to build dir.
	cp -r exporters           build
	cp -r samples             build
	mv    build/samples/core  build

	if [ "${PLATFORM}" = "linux" ]; then
		# Create script that launches the editor.
		rm build/crown
		{
			echo "#!/bin/sh"
			echo "PROJECT=\$1"
			echo "LEVEL=\$2"
			echo "if [ ! -z \$PROJECT ]; then"
			echo "	PROJECT=\$(readlink -f \${PROJECT})"
			echo "fi"
			# https://stackoverflow.com/questions/59895/how-to-get-the-source-directory-of-a-bash-script-from-within-the-script-itself
			echo "DIR=\"\$( cd \"\$( dirname \"\$0\" )\" >/dev/null 2>&1 && pwd )\""
			echo "cd \${DIR}/linux64/bin"
			echo "export UBUNTU_MENUPROXY="
			echo "./level-editor-release \${PROJECT} \${LEVEL}"
		} >> build/crown
		chmod +x build/crown
	elif [ "${PLATFORM}" = "windows" ]; then
		# Create script that launches the editor.
		rm build/crown.bat
		{
			echo "@echo off"
			echo "call :ABSOLUTEPATH %1"
			echo "set PROJECT=%RETVAL%"
			echo "set LEVEL=%2"
			echo ""
			echo "cd windows64\bin"
			echo "start level-editor-release.exe %PROJECT% %LEVEL%"
			echo ""
			echo "exit"
			echo ""
			echo ":ABSOLUTEPATH"
			echo "  SET RETVAL=%~dpfn1"
			echo "  EXIT /B"
		} >> build/crown.bat

		export MINGW=/mingw64
		export PATH="${PATH}:${MINGW}/bin"

		# Copy editor from MinGW build.
		cp build/mingw64/bin/level-editor-*.exe build/windows64/bin

		# Copy required DLLs.
		ldd build/mingw64/bin/level-editor-release.exe | grep '\/mingw.*\.dll' -o | xargs -I{} cp "{}" build/windows64/bin

		# Copy GTK-related executables.
		cp /mingw64/bin/fc-cache.exe                    build/windows64/bin
		cp /mingw64/bin/fc-cat.exe                      build/windows64/bin
		cp /mingw64/bin/fc-list.exe                     build/windows64/bin
		cp /mingw64/bin/fc-match.exe                    build/windows64/bin
		cp /mingw64/bin/fc-pattern.exe                  build/windows64/bin
		cp /mingw64/bin/fc-query.exe                    build/windows64/bin
		cp /mingw64/bin/fc-scan.exe                     build/windows64/bin
		cp /mingw64/bin/fc-validate.exe                 build/windows64/bin
		cp /mingw64/bin/gdbus.exe                       build/windows64/bin
		cp /mingw64/bin/gdk-pixbuf-query-loaders.exe    build/windows64/bin
		cp /mingw64/bin/gspawn-win64-helper-console.exe build/windows64/bin
		cp /mingw64/bin/gspawn-win64-helper.exe         build/windows64/bin
		cp /mingw64/bin/gtk-query-immodules-3.0.exe     build/windows64/bin
		cp /mingw64/bin/gtk-update-icon-cache.exe       build/windows64/bin

		# Copy additional DLLs.
		cp /mingw64/bin/liblzma-5.dll   build/windows64/bin
		cp /mingw64/bin/librsvg-2-2.dll build/windows64/bin
		cp /mingw64/bin/libxml2-2.dll   build/windows64/bin

		# Copy GDK pixbuf loaders.
		mkdir -p build/windows64/lib/gdk-pixbuf-2.0/2.10.0/loaders
		cp -r /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-*.dll build/windows64/lib/gdk-pixbuf-2.0/2.10.0/loaders
		cp -r /mingw64/lib/gdk-pixbuf-2.0/2.10.0/loaders.cache                 build/windows64/lib/gdk-pixbuf-2.0/2.10.0

		# Copy GLib schemas.
		mkdir -p build/windows64/share/glib-2.0/schemas
		cp -r /mingw64/share/glib-2.0/schemas/gschemas.compiled build/windows64/share/glib-2.0/schemas

		# Copy Adwaita and hicolor icons.
		mkdir -p build/windows64/share/icons/Adwaita/scalable
		cp -r /mingw64/share/icons/Adwaita/icon-theme.cache   build/windows64/share/icons/Adwaita
		cp -r /mingw64/share/icons/Adwaita/index.theme        build/windows64/share/icons/Adwaita
		cp -r /mingw64/share/icons/Adwaita/scalable/*         build/windows64/share/icons/Adwaita/scalable
		cp -r /mingw64/share/icons/hicolor                    build/windows64/share/icons
	fi
fi

# Strip unnecessary files from build dir.
rm -r build/mingw64                     2> /dev/null
rm -r build/projects                    2> /dev/null
rm -r build/"${BINARIES_DIR}"/obj/      2> /dev/null
rm    build/"${BINARIES_DIR}"/bin/*.a   2> /dev/null
rm    build/"${BINARIES_DIR}"/bin/*.exp 2> /dev/null
rm    build/"${BINARIES_DIR}"/bin/*.ilk 2> /dev/null
rm    build/"${BINARIES_DIR}"/bin/*.lib 2> /dev/null

for build_cfg in debug development release; do
	rm build/"${BINARIES_DIR}"/bin/bgfx-"${build_cfg}".*        2> /dev/null
	rm build/"${BINARIES_DIR}"/bin/bimg_decode-"${build_cfg}".* 2> /dev/null
	rm build/"${BINARIES_DIR}"/bin/bimg_encode-"${build_cfg}".* 2> /dev/null
	rm build/"${BINARIES_DIR}"/bin/bimg-"${build_cfg}".*        2> /dev/null
	rm build/"${BINARIES_DIR}"/bin/bullet-"${build_cfg}".*      2> /dev/null
	rm build/"${BINARIES_DIR}"/bin/bx-"${build_cfg}".*          2> /dev/null
done

rm "build/${BINARIES_DIR}/bin/shaderc-debug${EXE_SUFFIX}"                                                2> /dev/null
rm "build/${BINARIES_DIR}/bin/shaderc-development${EXE_SUFFIX}"                                          2> /dev/null
rv "build/${BINARIES_DIR}/bin/shaderc-release${EXE_SUFFIX}" "${BINARIES_DIR}/bin/shaderc${EXE_SUFFIX}"   2> /dev/null
rm "build/${BINARIES_DIR}/bin/texturec-debug${EXE_SUFFIX}"                                               2> /dev/null
rm "build/${BINARIES_DIR}/bin/texturec-development${EXE_SUFFIX}"                                         2> /dev/null
rv "build/${BINARIES_DIR}/bin/texturec-release${EXE_SUFFIX}" "${BINARIES_DIR}/bin/texturec${EXE_SUFFIX}" 2> /dev/null
rm "build/${BINARIES_DIR}/bin/level-editor-imgui-*"                                                      2> /dev/null
rm "build/${BINARIES_DIR}/bin/crown-debug${EXE_SUFFIX}"                                                  2> /dev/null

if [ "${PLATFORM}" = "android" ]; then
	rm -r build/"${BINARIES_DIR}"/bin/jit
fi

# Create release package from build dir.
mv build "${PACKAGENAME}"
${ZIP} "${TARBALLNAME}" "${PACKAGENAME}"

# Copy package to server.
if [ ! -z "${PACKAGESERVER}" ]; then
	if [ "${PLATFORM}" = "linux" ]; then
		ssh "${PACKAGESERVER}" "mkdir -p ~/Data/vms/${PACKAGENAME}"
	fi
	scp "${TARBALLNAME}" "${PACKAGESERVER}":~/Data/vms/"${PACKAGENAME}"
fi
