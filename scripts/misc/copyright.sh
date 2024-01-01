#!/bin/sh

# Updates copyright year in all text files in the tree, excluding 3rdpary and
# other special folders.
YEAR=$(date +"%Y")

find .                        \
	-not -path './.git/*'     \
	-not -path './3rdparty/*' \
	-not -path './build/*'    \
	-type f                   \
	| tr '\n' '\0'            \
	| xargs -0 -n1 sed -i "s/-[0-9]\{4\} Daniele Bartolini/-${YEAR} Daniele Bartolini/g"
