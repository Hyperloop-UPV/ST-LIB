#!/usr/bin/env bash
set -euo pipefail

if ! command -v pre-commit >/dev/null 2>&1; then
    echo "pre-commit no esta instalado."
    echo "Instala con: pip install pre-commit"
    exit 1
fi

pre-commit install --install-hooks --hook-type pre-commit --hook-type pre-push

echo "Hooks instalados (pre-commit y pre-push)."
