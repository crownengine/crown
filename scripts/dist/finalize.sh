#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

set -eu

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

TARGET_PLATFORMS="android-arm android-arm64 html5-wasm windows-x32"
MASTER_PLATFORMS="linux-x64 windows-x64"

merge_and_compress () {
	for mp in $MASTER_PLATFORMS; do
		# Merge target platforms into master ones.
		for tp in $TARGET_PLATFORMS; do
			cp -r "${PARTIAL_PKG}-${tp}"/* "${PARTIAL_PKG}-${mp}"
		done

		# Compress master platforms.
		if [ "${mp}" = "windows-x64" ]; then
			(cd "${PARTIALS_DIR}" && zip -rq "${PKG_NAME}-${mp}.zip" "${PKG_NAME}-${mp}")
		else
			(cd "${PARTIALS_DIR}" && tar -czf "${PKG_NAME}-${mp}.tar.gz" "${PKG_NAME}-${mp}")
		fi
	done
}

if [ ! -d "${PARTIALS_DIR}" ]; then
	echo "Partials dir not found."
	exit 1;
fi

merge_and_compress

# Move to output dir.
for zip in "${PARTIALS_DIR}"/*.gz "${PARTIALS_DIR}"/*.zip; do
	mv "${zip}" "${OUTPUT_DIR}"
done

rm -rf "${PARTIALS_DIR}" # So that sums.sh won't complain.
