# ST-LIB Build and Presets

## 1. Available Presets

```sh
cmake --list-presets
```

Main presets:

- `simulator`
- `simulator-asan`
- `nucleo-*`
- `board-*`

## 2. Simulator Build

```sh
cmake --preset simulator
cmake --build --preset simulator
```

With sanitizers:

```sh
cmake --preset simulator-asan
cmake --build --preset simulator-asan
```

## 3. MCU Build

Example:

```sh
cmake --preset board-debug-eth-ksz8041
cmake --build --preset board-debug-eth-ksz8041
```

## 4. Helper Scripts

Python wrapper:

```sh
python3 tools/build.py --preset simulator --run-tests
```

Shell wrapper:

```sh
./tools/build.sh simulator --run-tests
```
