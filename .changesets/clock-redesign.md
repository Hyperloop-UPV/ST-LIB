release: major
summary: Add ClockDomain, a centralised clock validator and applicator

Clock configuration is a precomputed `ClockTree` (from a host tool or hand-written). The firmware validates it at compile time and applies it at runtime.  No solver runs in the firmware.

- `ClockTree` stores only decisions (M/N/P/Q/R, prescalers, source enums); all frequencies are derived via accessors (`sysclk(t)`, `source_frequency(t, src)`, etc.)
- `Board<>` takes a mandatory `ClockTree` as the second template parameter
- Peripherals register clock requirements via `ClockDomain::Device` with typed models: `SPIClockModel<Group, MaxBaud, MinBaud>`, `ADCClockModel<MaxADCCLK>`, `SDClockModel<MaxFreq, MinFreq>`
- `ClockDomain::build()` validates PLL ranges, bus consistency, and peripheral requirements at compile time - refuses to compile on failure
- `ClockDomain::Init::init(tree)` applies the validated tree to HAL registers
- ADC prescaler computed from kernel clock at init (no longer user-specified)
- SPI/SD use mandatory frequency ranges
- Removed `HALconfig`
