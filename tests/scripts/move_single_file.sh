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

# simple file movement

file_before="before.txt"
echo "abc" > "$file_before"

file_after="after.txt"

"$BINARY" -D create "patchfile" -R "$file_before" "$file_after"
"$BINARY" -D apply "patchfile" .

[ "$(cat $file_after)" == "abc" ] || exit 1
[ ! -f "$file_before" ] && exit 0 || exit 1
