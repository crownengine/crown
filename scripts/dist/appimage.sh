#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

set -euo pipefail
shopt -s nullglob

GTK_PACKAGE=gtk+-3.0
GTK_ROOT=${GTK3_ROOT:-}
SCHEMA_PREFIX=org.gtk.Settings
if [ "${1:-}" = --gtk4 ]; then
	GTK_PACKAGE=gtk4
	GTK_ROOT=${GTK4_ROOT:-}
	SCHEMA_PREFIX=org.gtk.gtk4
	shift
fi

if [ $# -ne 1 ] || [ "$1" = "--help" ]; then
	echo "Usage: $0 [--gtk4] <crown-linux-x64.tar.gz>"
	echo "Create an AppImage beside the finalized Linux SDK archive."
	echo "Build on Ubuntu 22.04; set CXX to the compiler used to build the SDK."
	echo "Set GTK3_ROOT or GTK4_ROOT to activate and package a custom GTK SDK."
	exit 0
fi

ARCHIVE=$(realpath "$1")
PACKAGENAME=$(basename "${ARCHIVE}" .tar.gz)
case "${PACKAGENAME}" in
crown-*-linux-x64) ;;
*) echo "Expected a finalized Crown Linux x64 archive." >&2; exit 1 ;;
esac

SCRIPT_DIR=$(cd -- "$(dirname -- "$0")" && pwd)
OUTPUT_DIR=$(dirname "${ARCHIVE}")
if [ -n "${GTK_ROOT}" ]; then
	GTK_ROOT=$(realpath "${GTK_ROOT}")
	# shellcheck source=/dev/null
	. "${GTK_ROOT}/activate.sh"
else
	GTK_ROOT=$(realpath "$(pkg-config --variable=prefix "${GTK_PACKAGE}")")
fi
for package in "${GTK_PACKAGE}" gdk-pixbuf-2.0 gio-2.0; do
	if [ "$(realpath "$(pkg-config --variable=prefix "${package}")")" != "${GTK_ROOT}" ]; then
		echo "${package} must come from ${GTK_ROOT}." >&2
		exit 1
	fi
done
GTK_LIBDIR=$(pkg-config --variable=libdir "${GTK_PACKAGE}")
export PATH="${GTK_ROOT}/bin:${PATH}"
TOOLS_DIR=${XDG_CACHE_HOME:-${HOME}/.cache}/crown/appimage
WORK_DIR=$(mktemp -d)
trap 'rm -rf "${WORK_DIR}"' EXIT
APPDIR=${WORK_DIR}/Crown.AppDir
mkdir -p "${TOOLS_DIR}" "${APPDIR}/opt"
tar --no-same-owner -xzf "${ARCHIVE}" -C "${APPDIR}/opt"
SDK=${APPDIR}/opt/${PACKAGENAME}
test -x "${SDK}/crown-launcher"
test -x "${SDK}/platforms/linux64/bin/crown-editor-release"
test -d "${SDK}/core"
test -d "${SDK}/samples"
mv "${SDK}" "${APPDIR}/opt/crown"
SDK=${APPDIR}/opt/crown

download_file () {
	local name=$1
	local checksum=$2
	local url=$3
	if ! echo "${checksum}  ${TOOLS_DIR}/${name}" | sha256sum --check --status; then
		curl --fail --location --retry 5 --retry-all-errors "${url}" --output "${TOOLS_DIR}/${name}.tmp"
		echo "${checksum}  ${TOOLS_DIR}/${name}.tmp" | sha256sum --check
		mv "${TOOLS_DIR}/${name}.tmp" "${TOOLS_DIR}/${name}"
	fi
}

download_file appimagetool-x86_64.AppImage \
	ed4ce84f0d9caff66f50bcca6ff6f35aae54ce8135408b3fa33abfc3cb384eb0 \
	https://github.com/AppImage/appimagetool/releases/download/1.9.1/appimagetool-x86_64.AppImage
download_file runtime-x86_64 \
	2fca8b443c92510f1483a883f60061ad09b46b978b2631c807cd873a47ec260d \
	https://github.com/AppImage/type2-runtime/releases/download/20251108/runtime-x86_64
chmod +x "${TOOLS_DIR}/appimagetool-x86_64.AppImage"

# Host libraries from pkg2appimage's excludelist and Crown's platform exclusions.
# Keep libstdc++.so.6 and libgcc_s.so.1 bundled with the SDK.
EXCLUDED_LIBRARIES=(
	'ld-linux-x86-64.so.2*'
	'ld-linux.so.2*'
	'libBrokenLocale.so.1*'
	'libEGL.so.1*'
	'libGL.so.1*'
	'libGLX.so.0*'
	'libGLdispatch.so.0*'
	'libICE.so.6*'
	'libOpenGL.so.0*'
	'libSM.so.6*'
	'libX*.so*'
	'libX11-xcb.so.1*'
	'libX11.so.6*'
	'libanl.so.1*'
	'libapparmor.so*'
	'libasound.so*'
	'libasound.so.2*'
	'libasyncns.so*'
	'libavahi-*.so*'
	'libblkid.so*'
	'libc.so.6*'
	'libcap.so*'
	'libcidn.so.1*'
	'libcolord.so*'
	'libcom_err.so.2*'
	'libcups.so*'
	'libdbus-1.so*'
	'libdecor-0.so*'
	'libdl.so.2*'
	'libdrm.so.2*'
	'libexpat.so*'
	'libexpat.so.1*'
	'libfontconfig.so*'
	'libfontconfig.so.1*'
	'libfribidi.so*'
	'libfribidi.so.0*'
	'libgbm.so.1*'
	'libglapi.so.0*'
	'libgmp.so.10*'
	'libgpg-error.so.0*'
	'libgraphite2.so*'
	'libjack.so.0*'
	'libkeyutils.so*'
	'libm.so.6*'
	'libmount.so*'
	'libmvec.so.1*'
	'libnss_compat.so.2*'
	'libnss_dns.so.2*'
	'libnss_files.so.2*'
	'libnss_hesiod.so.2*'
	'libnss_nis.so.2*'
	'libnss_nisplus.so.2*'
	'libpipewire-0.3.so.0*'
	'libpthread.so.0*'
	'libpulse*.so*'
	'libresolv.so.2*'
	'librt.so.1*'
	'libselinux.so*'
	'libsystemd.so*'
	'libthread_db.so.1*'
	'libudev.so*'
	'libusb-1.0.so.0*'
	'libutil.so.1*'
	'libuuid.so*'
	'libuuid.so.1*'
	'libvulkan.so*'
	'libwayland*.so*'
	'libwayland-client.so.0*'
	'libxcb*.so*'
	'libxcb-dri2.so.0*'
	'libxcb-dri3.so.0*'
	'libxcb.so.1*'
	'libxkbcommon.so*'
)

# Extract appimagetool so packaging also works on runners without FUSE.
mkdir "${WORK_DIR}/appimagetool"
(cd "${WORK_DIR}/appimagetool" && "${TOOLS_DIR}/appimagetool-x86_64.AppImage" --appimage-extract > /dev/null)
APPIMAGETOOL=${WORK_DIR}/appimagetool/squashfs-root/AppRun

# Include the compiler runtimes used by the SDK. Platform libraries opened with
# dlopen() must come from the host, just as they do with the regular tarball.
CXX=${CXX:-g++}
mkdir -p "${APPDIR}/usr/bin" "${APPDIR}/usr/lib"
for lib in libstdc++.so.6 libgcc_s.so.1; do
	path=$("${CXX}" -print-file-name="${lib}")
	test -f "${path}"
	cp -L "${path}" "${APPDIR}/usr/lib/${lib}"
done
export LD_LIBRARY_PATH="${APPDIR}/usr/lib:${GTK_LIBDIR}:${GTK_ROOT}/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

mkdir -p "${APPDIR}/usr/share/icons"
cp -a "${GTK_ROOT}/share/icons/." "${APPDIR}/usr/share/icons"

# GTK3 loads input methods dynamically. Stage them and the image loaders
# so their dependencies are collected alongside the SDK executables.
QUERY_TOOLS=(gdk-pixbuf-query-loaders)
if [ "${GTK_PACKAGE}" = gtk4 ]; then
	mkdir -p "${APPDIR}/usr/share/gtk-4.0"
	cp -a "${GTK_ROOT}/share/gtk-4.0/emoji" "${APPDIR}/usr/share/gtk-4.0"
else
	GTK_BINARY_VERSION=$(pkg-config --variable=gtk_binary_version "${GTK_PACKAGE}")
	GTK_MODULEDIR=${APPDIR}/usr/lib/gtk-3.0/${GTK_BINARY_VERSION}
	mkdir -p "${GTK_MODULEDIR}"
	cp -a "${GTK_LIBDIR}/gtk-3.0/${GTK_BINARY_VERSION}/immodules" "${GTK_MODULEDIR}"
	QUERY_TOOLS+=(gtk-query-immodules-3.0)
fi
PIXBUF_MODULEDIR=${APPDIR}/usr/lib/gdk-pixbuf-2.0/2.10.0/loaders
mkdir -p "${PIXBUF_MODULEDIR}"
PIXBUF_LOADERS=("$(pkg-config --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0)/"*.so)
if [ ${#PIXBUF_LOADERS[@]} -gt 0 ]; then
	cp -a "${PIXBUF_LOADERS[@]}" "${PIXBUF_MODULEDIR}"
fi
SVG_LOADERS=("${PIXBUF_MODULEDIR}"/libpixbufloader[-_]svg.so)
if [ ${#SVG_LOADERS[@]} -eq 0 ]; then
	echo "Missing SVG image loader in ${GTK_ROOT}." >&2
	exit 1
fi

SCHEMAS_DIR=$(pkg-config --variable=datadir gio-2.0)/glib-2.0/schemas
mkdir -p "${APPDIR}/usr/share/glib-2.0/schemas"
cp "${SCHEMAS_DIR}/${SCHEMA_PREFIX}".*.gschema.xml "${APPDIR}/usr/share/glib-2.0/schemas"
"${GTK_ROOT}/bin/glib-compile-schemas" --strict "${APPDIR}/usr/share/glib-2.0/schemas"

# Query loaders at launch to produce caches containing the current mount path.
for tool in "${QUERY_TOOLS[@]}"; do
	path=$(find "${GTK_ROOT}/bin" "${GTK_LIBDIR}" -name "${tool}" -type f -print -quit)
	if [ -z "${path}" ]; then
		echo "Missing ${tool} in ${GTK_ROOT}." >&2
		exit 1
	fi
	cp -L "${path}" "${APPDIR}/usr/bin"
done

declare -A COLLECTED_FILES=()
collect_dependencies () {
	local binary libraries dependencies library path pattern
	binary=$(realpath "$1")
	if [ -n "${COLLECTED_FILES[${binary}]:-}" ]; then
		return
	fi
	COLLECTED_FILES[${binary}]=1
	if ! LC_ALL=C readelf -h "${binary}" 2>/dev/null | grep -q 'Class:.*ELF64'; then
		return
	fi
	libraries=$(LC_ALL=C readelf -d "${binary}" | sed -n 's/.*(NEEDED).*\[\([^]]*\)\].*/\1/p')
	if [ -z "${libraries}" ]; then
		return
	fi
	dependencies=$(LC_ALL=C ldd "${binary}")
	# ldd resolves the whole tree. Follow only direct dependencies so excluded
	# libraries do not pull their own dependencies into the AppImage.
	while read -r library; do
		for pattern in "${EXCLUDED_LIBRARIES[@]}"; do
			# shellcheck disable=SC2053 # Exclusions are glob patterns.
			if [[ "${library}" == ${pattern} ]]; then
				continue 2
			fi
		done
		path=$(printf '%s\n' "${dependencies}" | awk -v library="${library}" '$1 == library && $2 == "=>" && $3 ~ /^\// { print $3 }')
		if [ -z "${path}" ]; then
			printf 'Unresolved dependency %s for %s\n' "${library}" "${binary}" >&2
			return 1
		fi
		if [ ! -f "${APPDIR}/usr/lib/${library}" ]; then
			path=$(realpath "${path}")
			if [[ "${GTK_ROOT}" != /usr && "${path}" != "${GTK_ROOT}/"* ]]; then
				printf 'Dependency %s resolved outside %s: %s\n' "${library}" "${GTK_ROOT}" "${path}" >&2
				return 1
			fi
			cp -L "${path}" "${APPDIR}/usr/lib/${library}"
		fi
		collect_dependencies "${APPDIR}/usr/lib/${library}"
	done <<< "${libraries}"
}

collect_dependencies "${SDK}/crown-launcher"
while IFS= read -r -d '' binary; do
	collect_dependencies "${binary}"
done < <(
	find -L "${SDK}/platforms/linux64/bin" "${APPDIR}/usr/bin" -maxdepth 1 -type f -perm /111 -print0
	find -L "${APPDIR}/usr/lib" -type f -name '*.so*' -print0
)

mkdir -p "${APPDIR}/usr/share/applications" "${APPDIR}/usr/share/icons/hicolor/scalable/apps"
cp "${SDK}/org.crownengine.Crown.desktop" "${APPDIR}/usr/share/applications"
sed -i 's/^Exec=.*/Exec=AppRun/; s/^TryExec=.*/TryExec=AppRun/' "${APPDIR}/usr/share/applications/org.crownengine.Crown.desktop"
cp "${SDK}/org.crownengine.Crown.svg" "${APPDIR}/usr/share/icons/hicolor/scalable/apps"
ln -s usr/share/applications/org.crownengine.Crown.desktop "${APPDIR}"
ln -s usr/share/icons/hicolor/scalable/apps/org.crownengine.Crown.svg "${APPDIR}"

cp "${SCRIPT_DIR}/appimage/AppRun" "${APPDIR}/AppRun"
chmod +x "${APPDIR}/AppRun"
ARCH=x86_64 "${APPIMAGETOOL}" --runtime-file "${TOOLS_DIR}/runtime-x86_64" \
	"${APPDIR}" "${WORK_DIR}/${PACKAGENAME}.AppImage"
mv "${WORK_DIR}/${PACKAGENAME}.AppImage" "${OUTPUT_DIR}"
