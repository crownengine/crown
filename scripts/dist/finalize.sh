#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2024 Daniele Bartolini et al.

if [ $# -lt 1 ]; then
	echo "Usage: $0 <package-dir>"
	echo ""
	echo "e.g."
	echo "$0 path/to/crown-0.50.0"
	exit
fi

OUTPUT_DIR=$(realpath "$1")
PKG_NAME=$(basename "${OUTPUT_DIR}")
PARTIALS_DIR="${OUTPUT_DIR}/partials"
PARTIAL_PKG="${PARTIALS_DIR}/${PKG_NAME}"

TARGET_PLATFORMS="android-arm android-arm64 html5-wasm"
MASTER_PLATFORMS="linux-x64 windows-x64"

concatenate_and_compress () {
	for mp in $MASTER_PLATFORMS; do
		# Concatenate.
		for tp in $TARGET_PLATFORMS; do
			tar --concatenate --file       \
				"${PARTIAL_PKG}-${mp}.tar" \
				"${PARTIAL_PKG}-${tp}.tar"
		done

		# Compress.
		if [ "${mp}" = "windows-x64" ]; then
			tar xf "${PARTIAL_PKG}-${mp}.tar" --directory="${PARTIALS_DIR}"
			zip -rq "${PARTIAL_PKG}-${mp}.zip" "${PARTIAL_PKG}"
		else
			gzip "${PARTIAL_PKG}-${mp}.tar"
		fi
	done
}

if [ ! -d "${PARTIALS_DIR}" ]; then
	echo "Partials dir not found."
	exit 1;
fi

concatenate_and_compress

# Move to output dir.
for zip in "${PARTIALS_DIR}"/*.gz "${PARTIALS_DIR}"/*.zip; do
	mv "${zip}" "${OUTPUT_DIR}"
done
