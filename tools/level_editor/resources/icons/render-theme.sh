#!/bin/bash

INKSCAPE=inkscape
THEME="theme.svg"
THEME_INDEX="theme_index.txt"

for i in `cat $THEME_INDEX`
do
	for RESOLUTION in 16x16
	do
		echo "Rendering $RESOLUTION/emblems/$i-symbolic.symbolic.png"
		$INKSCAPE --export-id=$i \
			--export-id-only \
			--export-filename=$RESOLUTION/emblems/$i-symbolic.symbolic.png $THEME >/dev/null #\
	done
done

exit 0
