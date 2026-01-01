#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

crown_version_major () {
	grep "#define CROWN_VERSION_MAJOR" src/config.h | cut -d' ' -f3
}

crown_version_minor () {
	grep "#define CROWN_VERSION_MINOR" src/config.h | cut -d' ' -f3
}

crown_version_patch () {
	grep "#define CROWN_VERSION_PATCH" src/config.h | cut -d' ' -f3
}

crown_version_string () {
	echo "${1}.${2}.${3}"
}

# Prints the current version of Crown from config.h.
crown_version () {
	major=$(crown_version_major)
	minor=$(crown_version_minor)
	patch=$(crown_version_patch)
	crown_version_string ${major} ${minor} ${patch}
}

crown_set_version () {
	# Patch version numbers in config.h
	sed -i "s/CROWN_VERSION_MAJOR .*/CROWN_VERSION_MAJOR ${1}/g" src/config.h
	sed -i "s/CROWN_VERSION_MINOR .*/CROWN_VERSION_MINOR ${2}/g" src/config.h
	sed -i "s/CROWN_VERSION_PATCH .*/CROWN_VERSION_PATCH ${3}/g" src/config.h

	# Patch version string in config.vala
	sed -i "s/CROWN_VERSION_MAJOR = \".*/CROWN_VERSION_MAJOR = \"${1}\";/g" tools/config.vala
	sed -i "s/CROWN_VERSION_MINOR = \".*/CROWN_VERSION_MINOR = \"${2}\";/g" tools/config.vala
	sed -i "s/CROWN_VERSION_PATCH = \".*/CROWN_VERSION_PATCH = \"${3}\";/g" tools/config.vala
}

crown_docs_set_version () {
	# Patch version string in docs
	sed -i "s/version = '.*/version = '${1}.${2}.${3}'/g" docs/conf.py
	sed -i "s/release = '.*/release = '${1}.${2}.${3}'/g" docs/conf.py
}
