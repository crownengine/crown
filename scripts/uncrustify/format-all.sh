#!/bin/sh

find src/ -iname '*.h' -o -iname '*.inl' -o -iname '*.cpp' \
	| tr '\n' '\0'                                         \
	| xargs -0 -n1 ./scripts/uncrustify/uncrustify-wrapper.sh scripts/uncrustify/cpp.cfg CPP

find tools/ -iname '*.vala' -o -iname '*.c' \
	| tr '\n' '\0'                          \
	| xargs -0 -n1 ./scripts/uncrustify/uncrustify-wrapper.sh scripts/uncrustify/vala.cfg VALA
