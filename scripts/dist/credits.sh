#!/bin/sh

WEBSITE_DIR="../crown-website"
CREDITS_DIR="${WEBSITE_DIR}/src/data/about/credits"
ALIASES="scripts/dist/authors-aliases.csv"

CURR_TAG=$(git describe --tags --abbrev=0)
PREV_TAG=$(git describe --tags --abbrev=0 "$CURR_TAG"^)

scripts/dist/authors.sh -a "${ALIASES}" -s "$PREV_TAG" -e "$CURR_TAG" \
	| sort                                                            \
	| ./scripts/dist/to-json.sh                                       \
	> "${CREDITS_DIR}"/latest.json

scripts/dist/authors.sh -a "${ALIASES}" \
	| sort                              \
	| ./scripts/dist/to-json.sh         \
	> "${CREDITS_DIR}"/alltime.json
