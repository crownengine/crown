#! /bin/bash

# Usage:
# find src/ -iname *.h -o -iname *.inl -o -iname *.cpp | tr '\n' '\0' | xargs -0 -n1 ./code-format.sh 2> /dev/null

UNCRUSTIFY_CFG=$1

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

# Do uncrustify.
uncrustify () {
	../uncrustify/build/uncrustify               \
		-c $UNCRUSTIFY_CFG                       \
		-l $(basename -s '.cfg' $UNCRUSTIFY_CFG) \
		-q
}

uncrustify                                         \
	| fix_indentation_char                         \
	| add_newline_before_namespace_closing_bracket \
	| fix_semicolon_indentation
