#!/bin/sh

set -eu

print_help() {
	cat <<EOF
Usage: $0 [-a alias_file] [-s start_ref -e end_ref]

Options:
	-h             Show this help message and exit
	-a alias_file  Path to a file listing aliases (format: email,canonical_name).
                   Lines starting with '#' or empty lines are ignored.
	-s start_ref   Git ref (tag or commit) to start range (inclusive).
	-e end_ref     Git ref (tag or commit) to end range (inclusive).
EOF
}

ALIAS_FILE=""
START_REF=""
END_REF=""

# Parse options.
while getopts "a:s:e:h" opt; do
	case "$opt" in
	a)
		ALIAS_FILE="$OPTARG"
		;;
	s)
		START_REF="$OPTARG"
		;;
	e)
		END_REF="$OPTARG"
		;;
	h)
		print_help
		exit 0
		;;
	*)
		echo "Unknown option: -$OPTARG" >&2
		print_help
		exit 1
		;;
	esac
done

RANGE_SPEC=""
if [ -n "$START_REF" ] && [ -n "$END_REF" ]; then
	RANGE_SPEC="$START_REF..$END_REF"
elif [ -n "$START_REF" ] || [ -n "$END_REF" ]; then
	echo "Error: both -s start_ref and -e end_ref must be provided together" >&2
	exit 1
fi

git log ${RANGE_SPEC} --format='%an|%ae|%ad' --date=format:'%Y' |
awk -F'|' -v aliasfile="$ALIAS_FILE" '
BEGIN {
	# Load aliases if provided.
	if (aliasfile != "") {
		while ((getline line < aliasfile) > 0) {
			if (line ~ /^#/ || line == "")
				continue;
			split(line, arr, ",");
			alias_map[arr[1]] = arr[2];
		}
		close(aliasfile);
	}
}
{
	author = $1;
	email = $2;
	year = $3;

	# Skip invalid entries.
	if (author == "" || email == "" || year == "")
		next;

	# Determine canonical name by email alias or first-seen name.
	if (email in alias_map) {
		canonical = alias_map[email];
	} else if (!(email in name_map)) {
		name_map[email] = author; canonical = author;
	} else {
		canonical = name_map[email];
	}

	# Aggregate.
	counts[canonical]++;
	if (!(canonical in min_year) || year < min_year[canonical])
		min_year[canonical] = year;
	if (!(canonical in max_year) || year > max_year[canonical])
		max_year[canonical] = year;
}
END {
	# Emit count|canonical|years
	for (c in counts) {
		if (c == "")
			continue;

		start = min_year[c];
		end = max_year[c];
		years = (start == end ? start : start "–" end);
		print counts[c] "|" c "|" years;
	}
}' |
grep '|' |
awk -F'|' 'BEGIN { print "author,commits,years" } { print $2 "," $1 "," $3 }'
