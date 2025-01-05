#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2025 Daniele Bartolini et al.

if [ $# -lt 1 ]; then
	echo "Usage: $0 <package-dir>"
	echo ""
	echo "e.g."
	echo "$0 path/to/crown-0.50.0" # Create checksums for all files in v0.50.0
	exit;
fi

OUTPUT_DIR=$(realpath "$1")
SUMS="${OUTPUT_DIR}/SHA256SUMS"

if [ -f "${SUMS}" ]; then
    echo "${SUMS}"
    echo "Overwrite? [y/N]"
    read -r answer
    if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
		echo "Bye."
		exit;
    fi
fi

rm "${SUMS}"
(cd "${OUTPUT_DIR}" && sha256sum -- * > "${SUMS}" && cat "${SUMS}")
