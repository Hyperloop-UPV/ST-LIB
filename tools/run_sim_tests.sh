#!/bin/sh
set -eu

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"
build_dir="${repo_root}/out/build/simulator"

cmake -S "${repo_root}" -B "${build_dir}" -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build "${build_dir}"
ctest --test-dir "${build_dir}" --output-on-failure
