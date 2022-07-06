#!/bin/sh

format_src () {
	find src/ -iname '*.h' -o -iname '*.inl' -o -iname '*.cpp' \
		| tr '\n' '\0'                                         \
		| xargs -0 -n1 -P"$1" ./scripts/uncrustify/uncrustify-wrapper.sh scripts/uncrustify/cpp.cfg CPP
}

format_tools () {
	find tools/ -iname '*.vala' -o -iname '*.c' \
		| tr '\n' '\0'                          \
		| xargs -0 -n1 -P"$1" ./scripts/uncrustify/uncrustify-wrapper.sh scripts/uncrustify/vala.cfg VALA
}

NUM_JOBS=1
ENABLE_TOOLS=0

while [ $# -gt 0 ]; do
	case "$1" in
	-h|--help)
		echo "Usage: $0 [-j N] [--enable-tools]"
		exit 0
		;;
	-j|--jobs)
		NUM_JOBS=$2
		shift
		shift
		[ "${NUM_JOBS}" -eq "${NUM_JOBS}" ] 2>/dev/null || exit 1
		[ "${NUM_JOBS}" -gt 0 ] || exit 1
		;;
	--enable-tools)
		ENABLE_TOOLS=1
		shift
		;;
	-*)
		echo "Unknown option $1"
		exit 1
		;;
	*)
		;;
	esac
done

# Format all.
format_src "${NUM_JOBS}"
if [ "${ENABLE_TOOLS}" -eq 1 ]; then
	format_tools "${NUM_JOBS}"
fi
