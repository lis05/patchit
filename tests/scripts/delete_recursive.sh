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

# delete recursively

mkdir test
echo 123 > test/123
echo 456 > test/456
mkdir test/789
echo "mm" > test/789/mm

"$BINARY" -D create "patchfile" -D -r "test"
"$BINARY" -D apply "patchfile" .

[ ! -e "test" ] && exit 0 || exit 1
