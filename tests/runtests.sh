#!/bin/env bash

#set -o xtrace
set -o pipefail

BINARY=$(realpath "$1")

DATA_DIR="$(realpath tests/data)"
SCRIPTS_DIR="$(realpath tests/scripts)"
RUNTIME_DIR="$(realpath runtime_testing)"
RUNTIME_DATA="$(realpath runtime_data)"
LOGS_DIR="$(realpath logs)"

rm -rf "$LOGS_DIR"
mkdir "$LOGS_DIR"

rm -rf "$RUNTIME_DIR"
mkdir "$RUNTIME_DIR"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

success="1"

run_script() {
	local script_path="$1"
	local script_name="$(basename $script_path)"
	local log_file="$LOGS_DIR/$(basename $script_path).log"

	rm -rf "$RUNTIME_DIR/$script_name"
	mkdir "$RUNTIME_DIR/$script_name"

	pushd "$RUNTIME_DIR/$script_name" >/dev/null
	rm -rf "$RUNTIME_DATA"
	rsync -a "$DATA_DIR/" "$RUNTIME_DATA/"

	"$script_path" "$BINARY" "$RUNTIME_DIR/$script_name" "$RUNTIME_DATA" >"$log_file" 2>&1
	ec=$?

	if [ "$ec" = "0" ]; then
		echo -e "${GREEN}Passed${NC}: $(basename $script_path)"
	else
		success="0"
		echo -e "${RED}Failed${NC}: $(basename $script_path) ec=$ec"
		tail -n 15 "$log_file" | sed -e 's/^/     /'
		echo ""
	fi

	popd >/dev/null
}

echo "BINARY=$BINARY"
echo "DATA_DIR=$DATA_DIR"
echo "SCRIPTS_DIR=$SCRIPTS_DIR"
echo "RUNTIME_DIR=$RUNTIME_DIR"
echo "LOGS_DIR=$LOGS_DIR"
echo ""
echo "Running tests ======="

for script in $(ls $SCRIPTS_DIR); do
	run_script "$(realpath $SCRIPTS_DIR/$script)"
done

echo ""

[ "$success" = "1" ] && echo -e "${GREEN}TESTS PASSED${NC}" || echo -e "${RED}TESTS FAILED${NC}"
