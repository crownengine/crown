#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2025 Daniele Bartolini et al.

set -eu

. scripts/dist/version.sh

crown_docs_append_changelog_version () {
	VERSION_STRING=$(crown_version_string ${1} ${2} ${3})
	# Prepare CHANGELOG for next version.
	# Create a stable target to the heading based on version number.
	# Use the heading title to reference the very same target to avoid Sphinx discarding it.
	CHANGELOG_HEAD_TARGET=".. _v${VERSION_STRING}:"
	CHANGELOG_HEAD=":ref:\`${VERSION_STRING} --- DD MMM YYYY <v${VERSION_STRING}>\`"
	CHANGELOG_UNDERLINE=$(echo "${CHANGELOG_HEAD}" | sed 's/./-/g') # Replace all chars in CHANGELOG_HEAD with '-'

	{
		echo "Changelog"
		echo "========="
		echo ""
		echo "${CHANGELOG_HEAD_TARGET}"
		echo ""
		echo "${CHANGELOG_HEAD}"
		echo "${CHANGELOG_UNDERLINE}"
	} >> docs/changelog.rst.next

	tail docs/changelog.rst -n +3 >> docs/changelog.rst.next
	mv docs/changelog.rst.next docs/changelog.rst
}

if [ $# -ne 3 ]; then
	echo "Specify next version number."
	echo ""
	echo "Usage: $0 <major> <minor> <patch>"
	exit;
fi

# The new version to be bumped
VERSION_NEXT_MAJOR=$1
VERSION_NEXT_MINOR=$2
VERSION_NEXT_PATCH=$3
VERSION_NEXT=$(crown_version_string ${VERSION_NEXT_MAJOR} ${VERSION_NEXT_MINOR} ${VERSION_NEXT_PATCH})

crown_set_version $VERSION_NEXT_MAJOR $VERSION_NEXT_MINOR $VERSION_NEXT_PATCH
crown_docs_set_version $VERSION_NEXT_MAJOR $VERSION_NEXT_MINOR $VERSION_NEXT_PATCH
crown_docs_append_changelog_version $VERSION_NEXT_MAJOR $VERSION_NEXT_MINOR $VERSION_NEXT_PATCH

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
