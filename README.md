# ST-LIB

Library to simplify STM32 H7 firmware development using C++.

Main layers:

- HALAL
- ST-LIB_LOW
- ST-LIB_HIGH

## Quickstart

```sh
./tools/init-submodules.sh
cmake --preset simulator
cmake --build --preset simulator
ctest --preset simulator-all
```

## Documentation

- Setup: [`docs/setup.md`](docs/setup.md)
- Build and presets: [`docs/build-and-presets.md`](docs/build-and-presets.md)
- Testing: [`docs/testing.md`](docs/testing.md)

## Recommended Presets

- `simulator`
- `simulator-asan`
- `nucleo-*`
- `board-*`

List all presets:

```sh
cmake --list-presets
```

## Formatting and Hooks

```sh
pip install pre-commit
./tools/install-git-hooks.sh
pre-commit run --all-files
```
