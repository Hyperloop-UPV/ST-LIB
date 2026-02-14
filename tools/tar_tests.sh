#!/usr/bin/env bash
set -euo pipefail

archive_name="${1:-test_files.tar}"
preset_or_build_dir="${2:-simulator}"

if [[ "${preset_or_build_dir}" == */* ]]; then
    build_dir="${preset_or_build_dir%/}"
else
    build_dir="out/build/${preset_or_build_dir}"
fi

test_binary="${build_dir}/Tests/st-lib-test"

if [[ ! -f "${test_binary}" ]]; then
    echo "Test binary not found at '${test_binary}'. Build simulator tests first."
    exit 1
fi

tar -cvf "${archive_name}" "${test_binary}"
echo "Archive created: ${archive_name} (from ${test_binary})"
