#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2012-2026 Daniele Bartolini et al.

# Requires rsvg-convert and ImageMagick (magick).
# SVG output is a NUL-terminated Base64 string; pixels are top-down and tightly packed.

set -eu

usage()
{
	echo "Usage: $0 <input.svg> <symbol> <output.c> svg" >&2
	echo "       $0 <input.svg> <symbol> <output.c> <rgba|bgra> <width> <height> [pma]" >&2
	exit 1
}

if [ $# -lt 4 ] || [ $# -gt 7 ]; then
	usage
fi

INPUT=$1
SYMBOL=$2
OUTPUT=$3
FORMAT=$4

case "$SYMBOL" in
	''|[0-9]*|*[!a-zA-Z0-9_]*)
		echo "Invalid C symbol: ${SYMBOL}" >&2
		exit 1
		;;
esac

case "$FORMAT" in
	svg)
		[ $# -eq 4 ] || usage
		;;
	rgba|bgra)
		[ $# -ge 6 ] || usage
		WIDTH=$5
		HEIGHT=$6
		ALPHA=straight
		if [ $# -eq 7 ]; then
			[ "$7" = pma ] || usage
			ALPHA=premultiplied
		fi
		for DIMENSION in "$WIDTH" "$HEIGHT"; do
			case "$DIMENSION" in
				''|0*|*[!0-9]*)
					echo "Invalid dimension: ${DIMENSION}" >&2
					exit 1
					;;
			esac
		done
		;;
	*) echo "Invalid pixel format: ${FORMAT}" >&2; exit 1 ;;
esac

SCRIPT_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
TMP_DIR=$(mktemp -d "${OUTPUT}.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT
trap 'exit 1' HUP INT TERM

{
	printf '/* Generated with:\n *'
	for ARG in "$0" "$@"; do
		printf " '%s'" "$(printf '%s' "$ARG" | sed "s/'/'\\\\''/g")"
	done
	printf '\n */\n\n'
} > "$TMP_DIR/logo.c"
if [ "$FORMAT" = svg ]; then
	base64 < "$INPUT" > "$TMP_DIR/logo.base64"
	tr -d '\n' < "$TMP_DIR/logo.base64" > "$TMP_DIR/logo.svg.base64"
	printf '\000' >> "$TMP_DIR/logo.svg.base64"
	"$SCRIPT_DIR/bin2c.sh" "$TMP_DIR/logo.svg.base64" "${SYMBOL}_svg_base64" "$TMP_DIR/data.c"
	sed '1,2d' "$TMP_DIR/data.c" >> "$TMP_DIR/logo.c"
else
	rsvg-convert --width "$WIDTH" --height "$HEIGHT" --keep-aspect-ratio \
		--output "$TMP_DIR/logo.png" -- "$INPUT"

	set -- magick "$TMP_DIR/logo.png" -background none -gravity center -extent "${WIDTH}x${HEIGHT}"
	if [ "$ALPHA" = premultiplied ]; then
		set -- "$@" -channel RGB -fx 'u*u.a' +channel
	fi
	"$@" -depth 8 "${FORMAT}:$TMP_DIR/logo.pixels"
	"$SCRIPT_DIR/bin2c.sh" "$TMP_DIR/logo.pixels" "${SYMBOL}_pixels" "$TMP_DIR/data.c"
	{
		sed '1,2d' "$TMP_DIR/data.c"
		printf '\nconst unsigned int %s_width = %s;\n' "$SYMBOL" "$WIDTH"
		printf 'const unsigned int %s_height = %s;\n' "$SYMBOL" "$HEIGHT"
		printf 'const size_t %s_stride = (size_t)%s * 4;\n' "$SYMBOL" "$WIDTH"
	} >> "$TMP_DIR/logo.c"
fi

mv -- "$TMP_DIR/logo.c" "$OUTPUT"
