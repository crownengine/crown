#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2025 Daniele Bartolini et al.

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
	-b|--build)
		PKGVER=$2
		PKGREL=$3
		COMMITLOG="Crown v${PKGVER}-${PKGREL}"
		sed -i "s/^pkgver.*/pkgver=${PKGVER}/g" PKGBUILD
		sed -i "s/^pkgrel.*/pkgrel=${PKGREL}/g" PKGBUILD
		updpkgsums
		makepkg --printsrcinfo > .SRCINFO
		git add .SRCINFO PKGBUILD
		git commit -m "${COMMITLOG}"
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
		echo "2) $0 --build <pkgver> <pkgrel>"
		echo "3) $0 --publish"
		exit 0
		;;
	*)
		echo "Unknown option \`$1\`"
		exit 1
		;;
	esac
done
