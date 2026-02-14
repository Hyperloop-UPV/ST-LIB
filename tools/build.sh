#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <cmake-preset> [--run-tests]"
    exit 1
fi

preset="$1"
run_tests="${2:-}"

script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
repo_root="$(CDPATH= cd -- "${script_dir}/.." && pwd)"
cd "${repo_root}"

cmake --preset "${preset}"
cmake --build --preset "${preset}"

if [[ "${run_tests}" == "--run-tests" ]]; then
    case "${preset}" in
        simulator)
            ctest --preset simulator-all
            ;;
        simulator-asan)
            ctest --preset simulator-all-asan
            ;;
        *)
            echo "Preset '${preset}' does not support simulator tests"
            exit 1
            ;;
    esac
fi
