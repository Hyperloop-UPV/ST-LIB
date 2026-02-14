#!/usr/bin/env bash
set -euo pipefail

if ! command -v pre-commit >/dev/null 2>&1; then
    echo "pre-commit is not installed."
    echo "Install with: pip install pre-commit"
    exit 1
fi

pre-commit install --install-hooks --hook-type pre-commit --hook-type pre-push

echo "Hooks installed (pre-commit and pre-push)."
