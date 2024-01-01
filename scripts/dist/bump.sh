#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2024 Daniele Bartolini et al.

. scripts/dist/version.sh

if [ $# -ne 3 ]; then
	echo "Specify next version number."
	echo ""
	echo "Usage: $0 <major> <minor> <patch>"
	exit;
fi

git checkout master

# Read previous version
VERSION_PREV=$(git tag | tail -n 2 | head -n 1 | cut -c2-)

# Read current version
VERSION_MAJOR=$(crown_version_major)
VERSION_MINOR=$(crown_version_minor)
VERSION_PATCH=$(crown_version_patch)
VERSION=$(crown_version)

# The new version to be bumped
VERSION_NEXT_MAJOR=$1
VERSION_NEXT_MINOR=$2
VERSION_NEXT_PATCH=$3
VERSION_NEXT="${VERSION_NEXT_MAJOR}.${VERSION_NEXT_MINOR}.${VERSION_NEXT_PATCH}"
VERSION_NEXT_TITLE="${VERSION_NEXT} --- DD MMM YYYY"
VERSION_NEXT_UNDERLINE=$(echo "${VERSION_NEXT_TITLE}" | sed 's/./-/g') # Replace all chars in VERSION_NEXT_TITLE with '-'

# Patch version numbers in config.h
sed -i "s/MAJOR ${VERSION_MAJOR}/MAJOR ${VERSION_NEXT_MAJOR}/g" src/config.h
sed -i "s/MINOR ${VERSION_MINOR}/MINOR ${VERSION_NEXT_MINOR}/g" src/config.h
sed -i "s/PATCH ${VERSION_PATCH}/PATCH ${VERSION_NEXT_PATCH}/g" src/config.h
# Patch version string in config.vala
sed -i "s/${VERSION}/${VERSION_NEXT}/g" tools/config.vala
# Patch version string in docs
sed -i "s/${VERSION}/${VERSION_NEXT}/g" docs/conf.py

# Prepare CHANGELOG for next version
{
	echo "Changelog"
	echo "========="
	echo ""
	echo "${VERSION_NEXT_TITLE}"
	echo "${VERSION_NEXT_UNDERLINE}"
} >> docs/changelog.rst.next

tail docs/changelog.rst -n +3 >> docs/changelog.rst.next
mv docs/changelog.rst.next docs/changelog.rst

echo "Crown v${VERSION_NEXT} will be bumped"
echo "Continue? [y/N]"
read -r answer
if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
	echo "Bye"
	exit;
fi

# Commit changes
git add src/config.h
git add tools/config.vala
git add docs/conf.py
git add docs/changelog.rst
git commit -m "Bump Crown v${VERSION_NEXT}"
