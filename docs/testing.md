# ST-LIB Testing

## 1. Run All Tests (simulator)

```sh
cmake --preset simulator
cmake --build --preset simulator
ctest --preset simulator-all
```

## 2. Run a Test Subset

ADC only:

```sh
ctest --preset simulator-adc
```

## 3. Run Tests with Sanitizers

```sh
cmake --preset simulator-asan
cmake --build --preset simulator-asan
ctest --preset simulator-all-asan
```

## 4. Quick Script

```sh
./tools/run_sim_tests.sh
```

## 5. CI

Main workflows:

- `Compile Checks`
- `Run tests`
- `Format Checks`
