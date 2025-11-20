#!/bin/env bash


set -o xtrace
set -o pipefail
set -e

BINARY="$1"
RUNTIME_DIR="$2"

if [ "$(pwd)" != "$RUNTIME_DIR" ]; then
	echo "INVALID PWD: $(pwd), not $RUNTIME_DIR"
	exit 1
fi

# simple file modification

file_before="before.txt"
echo "abc" > "$file_before"

file_after="after.txt"
echo "abcdef" > "$file_after"

"$BINARY" -D create "patchfile" -M "$file_before" "$file_after"
"$BINARY" -D apply "patchfile" .

[ "$(cat $file_before)" == "abcdef" ] && exit 0 || exit 1
