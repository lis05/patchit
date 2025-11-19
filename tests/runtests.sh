#!/bin/env bash

#set -o xtrace

BINARY=$(realpath "$1")

DATA_DIR="$(realpath tests/data)"
SCRIPTS_DIR="$(realpath tests/scripts)"
RUNTIME_DIR="$(realpath runtime_testing)"
LOGS_DIR="$(realpath testing_logs)"

rm -rf "$LOGS_DIR"
mkdir "$LOGS_DIR"

run_script() {
	local script_path="$1"
	local log_file="$LOGS_DIR/$(basename $script_path).log"

	rm -rf "$RUNTIME_DIR"
	mkdir "$RUNTIME_DIR"

	rsync -a "$DATA_DIR/" "$RUNTIME_DIR/"

	pushd "$RUNTIME_DIR" >/dev/null

	bash -c "$script_path" "$BINARY" "$RUNTIME_DIR" 1>"$log_file" 2&>1 \
		&& echo "Passed: $(basename $script_path)" \
		|| echo "Failed: $(basename $script_path) ec=$?"

	popd >/dev/null
}

echo "Running tests ======="
for script in $(ls $SCRIPTS_DIR); do
	run_script "$(realpath $SCRIPTS_DIR/$script)"
done

