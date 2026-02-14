#!/usr/bin/env python3
import argparse
import subprocess
import sys
from pathlib import Path


TEST_PRESET_BY_CONFIGURE = {
    "simulator": "simulator-all",
    "simulator-asan": "simulator-all-asan",
}


REPO_ROOT = Path(__file__).resolve().parent.parent


def run_cmd(cmd: list[str]) -> int:
    return subprocess.call(cmd, cwd=REPO_ROOT)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        prog="build.py",
        description="Configure/build ST-LIB using CMake presets",
    )
    parser.add_argument(
        "-p",
        "--preset",
        default="simulator",
        help="CMake configure/build preset to use",
    )
    parser.add_argument(
        "--run-tests",
        action="store_true",
        help="Run tests after build when preset supports simulator tests",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    print(f"[build.py] Repository: {REPO_ROOT}")
    print(f"[build.py] Configure preset: {args.preset}")

    if run_cmd(["cmake", "--preset", args.preset]) != 0:
        return 1

    if run_cmd(["cmake", "--build", "--preset", args.preset]) != 0:
        return 1

    if args.run_tests:
        test_preset = TEST_PRESET_BY_CONFIGURE.get(args.preset)
        if not test_preset:
            print(f"[build.py] Preset '{args.preset}' does not have an associated test preset.")
            return 1
        if run_cmd(["ctest", "--preset", test_preset]) != 0:
            return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
