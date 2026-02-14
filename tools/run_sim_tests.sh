#!/bin/sh
set -eu

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"

cd "${repo_root}"
cmake --preset simulator
cmake --build --preset simulator
ctest --preset simulator-all
