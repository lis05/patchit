#!/bin/env bash


set -o xtrace
set -o pipefail
set -e

BINARY="$1"
RUNTIME_DIR="$2"
DATA="$3"

if [ "$(pwd)" != "$RUNTIME_DIR" ]; then
	echo "INVALID PWD: $(pwd), not $RUNTIME_DIR"
	exit 1
fi

# files in $DATA/input/nested_directories and $DATA/output/nested_directories
# a.txt, dir1/b.txt, dir1/dir2/c.txt

"$BINARY" -D create "patchfile" \
	-M -c zlib "$DATA/input/nested_directories/a.txt" \
		"$DATA/output/nested_directories/a.txt" \
	-M "$DATA/input/nested_directories/dir1/b.txt" \
		"$DATA/output/nested_directories/dir1/b.txt" \
	-M "$DATA/input/nested_directories/dir1/dir2/c.txt" \
		"$DATA/output/nested_directories/dir1/dir2/c.txt" \

"$BINARY" -D apply "patchfile" "$DATA/input/nested_directories"

diff -ra "$DATA/input/nested_directories" "$DATA/output/nested_directories"
ec=$?

[ "$ec" == "0" ] && exit 0 || exit 1
