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

# delete single file

file_before="before.txt"
echo "abc" > "$file_before"

"$BINARY" -D create "patchfile" -D "$file_before"
"$BINARY" -D apply "patchfile" .

[ ! -f "$file_before" ] && exit 0 || exit 1
