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

# three files in the same directory

file1_before="before1.txt"
echo "file1_before" > "$file1_before"

file2_before="before2.txt"
echo "file2_before" > "$file2_before"

file3_before="before3.txt"
echo "file3_before" > "$file3_before"

file1_after="after1.txt"
echo "after1" > "$file1_after"

file2_after="after2.txt"
echo "after2" > "$file2_after"

file3_after="after3.txt"
echo "after3" > "$file3_after"

"$BINARY" -D create "patchfile" -M "$file1_before" "$file1_after" \
								-M "$file2_before" "$file2_after" \
								-M "$file3_before" "$file3_after"
"$BINARY" -D apply "patchfile" .

[ "$(cat $file1_before)" == "after1" ] || exit 1
[ "$(cat $file2_before)" == "after2" ] || exit 1
[ "$(cat $file3_before)" == "after3" ] || exit 1
exit 0
