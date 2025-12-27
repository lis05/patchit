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

# modify and move file

file_before="before.txt"
echo "abc" > "$file_before"

file_modified="modified.txt"
echo "abcdef" > "$file_modified"

file_after="after.txt"

"$BINARY" -D create "patchfile" \
		-M "$file_before" "$file_modified" \
		-R "$file_modified" "$file_after"
"$BINARY" -D apply "patchfile" .

[ "$(cat $file_after)" == "abcdef" ] || exit 1
[ ! -f "$file_modified" ] && exit 0 || exit 1
