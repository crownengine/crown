#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2025 Daniele Bartolini et al.

if [ $# -gt 1 ]; then
	echo "Usage: $0 [version]"
	echo ""
	exit;
fi

VERSION=$1
if [ -z "${VERSION}" ]; then
	VERSION=$(git tag | tail -n 1)
elif [ "${VERSION}" = "master" ]; then
	VERSION="master"
else
	echo "Invalid version name"
	exit 1
fi

echo "Docs \`${VERSION}\` will be released"
echo "Continue? [y/N]"
read -r answer
if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
	echo "Bye."
	exit;
fi

# Switch to desired tag.
git checkout "${VERSION}"

# Build docs.
make clean
make docs

# Update gh-pages branch.
git checkout gh-pages
rm -r html/"${VERSION}"
mkdir html/"${VERSION}"
cp -r build/docs/html/* html/"${VERSION}"

# Update 'latest' only in stable releases.
if [ "${VERSION}" != "master" ]; then
	rm -r html/latest
	mkdir html/latest
	cp -r build/docs/html/* html/latest
	git add html/latest
fi

# Commit changes.
git add html/"${VERSION}"
git commit -m "Docs ${VERSION}"
