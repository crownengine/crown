#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2025 Daniele Bartolini et al.

crown_version_major () {
	grep "#define CROWN_VERSION_MAJOR" src/config.h | cut -d' ' -f3
}

crown_version_minor () {
	grep "#define CROWN_VERSION_MINOR" src/config.h | cut -d' ' -f3
}

crown_version_patch () {
	grep "#define CROWN_VERSION_PATCH" src/config.h | cut -d' ' -f3
}

# Prints the current version of Crown from config.h.
crown_version () {
	major=$(crown_version_major)
	minor=$(crown_version_minor)
	patch=$(crown_version_patch)
	echo "${major}.${minor}.${patch}"
}
