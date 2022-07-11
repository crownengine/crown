#!/bin/sh

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

if [ "${OSTYPE}" = "linux-gnu" ]; then
	OS="linux"
elif [ "${OSTYPE}" = "msys" ]; then
	OS="windows"
else
	OS="linux"
fi

if [ -z "${UNCRUSTIFY}" ]; then
	UNCRUSTIFY=./scripts/uncrustify/bin/"${OS}"/uncrustify
fi

# Do uncrustify.
echo "$3"
${UNCRUSTIFY} -q -c "$1" -l "$2" -f "$3"           \
	| fix_indentation_char                         \
	| add_newline_before_namespace_closing_bracket \
	| fix_semicolon_indentation                    \
	> "$3".new                                     \
	&& mv "$3".new "$3"
