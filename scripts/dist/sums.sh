#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2023 Daniele Bartolini et al.

SUMS="SHA256SUMS"

if [ $# -lt 1 ]; then
	echo "Usage: $0 <packages-dir>"
	echo ""
	echo "e.g."
	echo "$0 crown-0.41.0" # Create checksums for all files in v0.41.0
	exit;
fi

if [ -f "$1/${SUMS}" ]; then
    echo "$1/${SUMS} exists."
    echo "Overwrite? [y/N]"
    read -r answer
    if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
		echo "Bye."
		exit;
    fi
fi

rm "$1/${SUMS}"
(cd "$1" && sha256sum ./* > "${SUMS}")
cat "$1/${SUMS}"
