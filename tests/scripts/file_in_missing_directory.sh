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

# simple file modification in directory a/b/c/d/e
# the file is empty because at the moment of writing
# this test patchit does not support diffs that copy
# entire files.

mkdir -p "a/b/c/d/e"
touch "a/b/c/d/e/file.txt"

echo "after" > after.txt

"$BINARY" -D create "patchfile" -M -e -p "a/b/c/d/e/file.txt" "after.txt"

rm -rf "a/"
"$BINARY" -D apply "patchfile" .

[ "$(cat a/b/c/d/e/file.txt)" == "after" ] && exit 0 || exit 1
