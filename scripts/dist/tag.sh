#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2025 Daniele Bartolini et al.

. scripts/dist/version.sh

VERSION=$(crown_version)

# Read current date of release.
TODAY=$(LC_ALL=en_US.utf8 date '+%d %b %Y')

# Update CHANGELOG version and date.
sed -i "s/DD MMM YYYY/${TODAY}/g" docs/changelog.rst

echo "v${VERSION} will be tagged now (${TODAY})"
echo "Continue? [y/N]"
read -r answer
if [ "${answer}" != "y" ] && [ "${answer}" != "Y" ]; then
	echo "Bye."
	exit;
fi

# Commit changes.
git add docs/changelog.rst
git commit -m "Crown v${VERSION}"
git tag "v${VERSION}"
