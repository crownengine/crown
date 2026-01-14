#!/bin/bash
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

set -eu

. scripts/dist/version.sh

NOCONFIRM=0
ARGS=()

while [ $# -gt 0 ]; do
	case "$1" in
	-h|--help)
		echo "Usage: $0 [options] [version]"
		echo ""
		echo "Options:"
		echo "  --noconfirm  Skip any user confirmations."
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
		ARGS+=("$1")
		shift
		;;
	esac
done

set -- "${ARGS[@]}"

VERSION=${1-}
VERSION_PATCH=$(crown_version_patch)
if [ -z "${VERSION}" ]; then
	VERSION=$(crown_version)
	VERSION_DIR=v$(crown_version_major).$(crown_version_minor).0
elif [ "${VERSION}" = "master" ]; then
	VERSION="master"
	VERSION_DIR="master"
else
	echo "Invalid version name"
	exit 1
fi

if [ "${NOCONFIRM}" -eq 0 ]; then
	echo "Docs \`${VERSION}\` will be released"
	echo "Continue? [y/N]"
	read -r answer
	if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
		echo "Bye."
		exit;
	fi
fi

# Build docs.
make clean
make docs

# Update gh-pages branch.
git checkout gh-pages
rm -rf html/"${VERSION_DIR}"
mkdir html/"${VERSION_DIR}"
cp -r build/docs/html/* html/"${VERSION_DIR}"

# Update 'latest' only in stable releases, excluding patches.
if [ "${VERSION}" != "master" ] && [ "${VERSION_PATCH}" -eq 0 ]; then
	rm -rf html/latest
	mkdir html/latest
	cp -r build/docs/html/* html/latest
	git add html/latest
fi

# Commit changes.
git add html/"${VERSION_DIR}"
git commit -m "Docs ${VERSION}"
