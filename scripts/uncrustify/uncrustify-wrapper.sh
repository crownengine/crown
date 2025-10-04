#!/bin/sh

set -eu

# Convert leading spaces to tabs.
fix_indentation_char () {
	unexpand --tabs=4 --first-only
}

# Add newline before namespace's closing bracket.
add_newline_before_namespace_closing_bracket () {
	awk '                        \
		/} \/[\/\*] namespace/ { \
			print '\n';          \
		}                        \
		{                        \
			print $0;            \
		}                        \
	'
}

# Fix the indentation of ; character when it is alone on a line.
fix_semicolon_indentation () {
	awk '
		{                                                      \
			if ($0 ~ /[\t];$/) {                               \
				output_str = "";                               \
				last_indent_level = gsub(/\t/, "", last_line); \
                                                               \
				for (ii = 0; ii < last_indent_level; ++ii)     \
					output_str = output_str "\t";              \
				output_str = output_str ";";                   \
                                                               \
				print output_str;                              \
			} else {                                           \
				print $0                                       \
			}                                                  \
		}                                                      \
		{                                                      \
			last_line = $0;                                    \
		}                                                      \
	'
}

if [ "${OSTYPE-}" = "linux-gnu" ]; then
	OS="linux"
elif [ "${OSTYPE-}" = "msys" ]; then
	OS="windows"
else
	OS="linux"
fi

UNCRUSTIFY_INTERNAL=./scripts/uncrustify/bin/"${OS}"/uncrustify

if [ -n "$2" ]; then
	# Do uncrustify.
	echo "$2"
	TEMPFILE_UNCRUSTIFY=$(mktemp)
	if ! ${UNCRUSTIFY-$UNCRUSTIFY_INTERNAL} -q -c "$1" -f "$2" > "$TEMPFILE_UNCRUSTIFY"; then
		echo "Failed to format '$2'"
		exit 1
	else
		TEMPFILE_AWK=$(mktemp)
		fix_indentation_char < "$TEMPFILE_UNCRUSTIFY"  \
		| add_newline_before_namespace_closing_bracket \
		| fix_semicolon_indentation                    \
		> "$TEMPFILE_AWK"

		rm "$TEMPFILE_UNCRUSTIFY"

		# Only overwrite if there are differences.
		if ! cmp -s "$2" "$TEMPFILE_AWK"; then
			mv "$TEMPFILE_AWK" "$2"
		else
			rm "$TEMPFILE_AWK"
		fi
	fi
fi
