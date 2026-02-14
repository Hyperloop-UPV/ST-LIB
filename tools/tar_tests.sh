#!/usr/bin/env bash
set -euo pipefail

archive_name="${1:-test_files.tar}"
test_binary="out/build/simulator/Tests/st-lib-test"

if [[ ! -f "${test_binary}" ]]; then
    echo "Test binary not found at '${test_binary}'. Build simulator tests first."
    exit 1
fi

tar -cvf "${archive_name}" "${test_binary}"
echo "Archive created: ${archive_name}"
