#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

set -eu

commit_package()
{
	COMMITLOG=$1
	updpkgsums
	makepkg --printsrcinfo > .SRCINFO
	git add .SRCINFO PKGBUILD
	git commit -m "${COMMITLOG}"
}

while true; do
	case "$1" in
	-c|--clean)
		echo "Cleaning..."
		rm -rf pkg 2> /dev/null
		rm -rf src 2> /dev/null
		rm crown-*.tar.gz 2> /dev/null
		rm crown-*.tar.xz 2> /dev/null
		exit 0
		;;
	-b|--build|--build-release)
		PKGVER=$2
		PKGREL=$3
		COMMITLOG="Crown v${PKGVER}-${PKGREL}"
		sed -i "s/^pkgver.*/pkgver=${PKGVER}/g" PKGBUILD
		sed -i "s/^pkgrel.*/pkgrel=${PKGREL}/g" PKGBUILD
		commit_package "${COMMITLOG}"
		exit $?
		;;
	-n|--build-nightly)
		PKGVER=$2
		PKGREL=$3
		UPSTREAM_VERSION=$4
		COMMIT=$5
		GDRIVE_ID=$6
		COMMITLOG="Crown Nightly v${PKGVER}-${PKGREL}"
		sed -i "s/^pkgver.*/pkgver=${PKGVER}/g" PKGBUILD
		sed -i "s/^pkgrel.*/pkgrel=${PKGREL}/g" PKGBUILD
		sed -i "s/^_upstream_version.*/_upstream_version=${UPSTREAM_VERSION}/g" PKGBUILD
		sed -i "s/^_commit.*/_commit=${COMMIT}/g" PKGBUILD
		sed -i "s/^_gdrive_id.*/_gdrive_id=${GDRIVE_ID}/g" PKGBUILD
		commit_package "${COMMITLOG}"
		exit $?
		;;
	-f|--force)
		makepkg -f
		exit $?
		;;
	-p|--publish)
		git push
		exit $?
		;;
	-h|--help)
		echo "Usage:"
		echo "1) $0 --clean"
		echo "2) $0 --build-release <pkgver> <pkgrel>"
		echo "3) $0 --build-nightly <pkgver> <pkgrel> <upstream-version> <commit> <gdrive-file-id>"
		echo "4) $0 --publish"
		exit 0
		;;
	*)
		echo "Unknown option \`$1\`"
		exit 1
		;;
	esac
done
