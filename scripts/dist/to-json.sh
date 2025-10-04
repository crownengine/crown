#!/bin/sh

set -eu

# If a filename is provided use it, otherwise read from stdin.
CSV=${1:-/dev/stdin}

awk -F, '
	NR==1 {
		# Store header names.
		for (i=1; i<=NF; i++) header[i]=$i;
		print "["
	}
	NR>1 {
		printf "%s{", sep;
		for (i=1; i<=NF; i++) {
			printf "%s\"%s\":\"%s\"", (i>1?",":""), header[i], $i;
		}
		printf "}"
		sep = ",\n"
	}
	END {
		print "\n]"
	}
' "$CSV"
