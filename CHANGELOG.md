# Changelog

This file tracks ST-LIB releases prepared by the semiautomated release flow.
The revived semantic-versioning baseline starts at `v5.0.0`.

Historical releases that predate this file remain available in Git tags such as
`v1.0.0`, `v3.0.0`, `v4.0.0-beta`, and `h10`.

## v7.0.0 - 2026-10-03

### Breaking Changes

- Refactor linker script, startup code, and memory model with unified copy/zero tables, configurable ITCM, and weak BoardInit. Also fixes LTO.
  ## Implemented
  
  ### Linker Script (`LinkerScript.ld`)
  - **Unified copy table**: all initialized data sections (ISR vector, ITCM code, `.data`, `.dtcm_rodata`, D1/D2/D3 NC and cached data/rodata) are copied from FLASH to RAM by a single generic loop in `Reset_Handler`. No more hand-written copy per section.
  - **Unified zero table**: all BSS sections (DTCM `.bss`, D1/D2/D3 NC and cached BSS) are zero-initialized by the same generic mechanism.
  - **Configurable ITCM**: `__ITCM_SIZE` and `__ITCM_BUILD` macros (passed via C preprocessor `-D` at build time) control ITCM size and whether `.text` is placed in ITCM instead of FLASH.
  - **ISR vector always in ITCM**: the interrupt vector table is unconditionally placed in ITCMRAM and copied at startup.
  - **DTCM constants**: `.dtcm_rodata` section for constants explicitly placed in DTCM (via `DTCM_RODATA` / `DTCM_RODATA_INLINE` macros), initialized from FLASH by the copy table.
  - **Initialized data for all memory domains**: D1, D2, D3 each have non-cached and cached data/rodata sections (`>RAM_Dx AT> FLASH`) copied by the copy table.
  - **Null-pointer guard**: 32-byte MPU region at `0x00000000` (NO_ACCESS) catches runtime null dereferences. Vector table relocated to a 1024-byte aligned location after the 32 security bytes, loaded in VTOR via `.null_guard` NOLOAD section. Region 12 > region 11 → takes priority.
  - **LMA collision guard**: `BYTE(0)` in every `AT> FLASH` section prevents empty sections from clustering at the same PhysAddr, suppressing STM32_Programmer "overlapping segments" warnings.
  
  ### Startup Code (`StartupCode.s`)
  - Replaced per-section manual copy with generic copy-table loop (`__copy_table_start` → `__copy_table_end`).
  - Replaced per-section manual BSS zeroing with generic zero-table loop (`__zero_table_start` → `__zero_table_end`).
  - Removed `Reset_Handler` reliance on hardcoded section symbols; everything is table-driven.
  - Calls `weak BoardInit()` between `SystemInit` and global constructors, allowing early hardware initialization from C++.
  
  ### System / Board Init (`System.c`)
  - Moved from template-project to ST-LIB so all projects benefit.
  - Provides `weak BoardInit()` and `weak ConfigurationChecker()` — user overrides in firmware.
  
  ### MPU (`MPU.hpp`)
  - Fixed `MPUDomain::Instance::as()` and `construct()`: `Request::e` → `Target.e` for valid constexpr member access.
  - Fixed `static_assert` logic: `is_nc && volatile` → `!is_nc || volatile` (cached buffers no longer trigger the assert).
  - Added null-pointer guard region (region 12, `0x00000000`, 32 bytes, NO_ACCESS).
  - Inline specifiers for all `*_INLINE` macros (since inline things are placed in special COMDAT sections that must be separate one from another and everything else).
  
  ## Not implemented (left as weak symbols)
  - `ConfigurationChecker`: a function to validate ITCM size and other build-time configuration at runtime. User can define it to add custom startup checks.
  
  ## Migration guide (template-project)
  The corresponding template-project PR adapts `BoardInit` to the new weak function contract, and changes the CMAKE as needed. See the template-project for details.

### Features

- Rework SNTP module in ST-LIB to use ST-LIB scheduler instead of Lwip one
  example usage:
  
  ```cpp
  /* ... */
  int main(void) {
      MainBoard::init();
  
      led_instance = &MainBoard::instance_of<led>();
      auto eth_instance = &MainBoard::instance_of<eth>();
  
      Scheduler::register_task(200'000, []{ led_instance->toggle(); });
  
      ST_LIB::SNTP::start("192.168.0.9");
  
      Scheduler::register_task(1'000'000, []{
          RTCData data = Global_RTC::get_rtc_timestamp();
          INFO("RTC: %u; %uyr %umon %uday %uh %umin %us",
              data.counter, data.year, data.month, data.day,
              data.hour, data.minute, data.second);
      });
  
      while (1) {
          eth_instance->update();
          Scheduler::update();
          Diagnostics::Hub::flush();
      }
  }
  ```
  Led is not needed but it is to show the board is doing work.

### Fixes

- fix 1k* time scaling factor not needed

## v6.3.1 - 2026-07-15

### Fixes

- Retry pending FAULT order propagation to disconnected peers via check_transitions()
  When propagate_fault() fails to send the FAULT order (e.g. TCP connection not yet
  established), the target is marked pending. check_transitions() automatically
  retries pending targets on each call while the board remains faulted, until the
  send succeeds.

## v6.3.0 - 2026-07-13

### Features

- Add fault propagation to FaultController for sending FAULT order to remote peers with single-shot loop prevention via the `!faulted` guard
  Star-topology design: the `!faulted` guard in `request_fault()` is the only loop breaker. Each board propagates exactly once on the `!faulted → faulted` transition, so no `fault_received_from_peer` flag is needed. Works for any node faulting first (local fault or CS-commanded fault).

### Fixes

- try to fix input capture by reducing noise in interrupt callback and add hardware filtering when configuring the input capture
- EXTI does not turn on automatically now. It needs to be done via turn_on()
  Optional extra context in markdown.

## v6.2.0 - 2026-07-10

### Features

- Add variable-length TCP orders, diagnostic forwarding, and flush throttling
  - `TcpOrderStreamParser` now supports variable-length orders: when an order's `get_size()` returns 0, all remaining stream data is passed to `parse()`, and the post-parse size is used for byte consumption. A `reset_for_receive()` hook is called after consumption so orders can reset state for the next packet.
  - `Order` gains a virtual `reset_for_receive()` no-op that subclasses can override.
  - `Socket` constructor no longer panics on `ERR_RTE` during `tcp_connect()` — sets `pending_connection_reset` instead, letting the connection retry when the route becomes available.
  - `OrderProtocolDiagnosticSink` now accepts a target `OrderProtocol*` via its constructor, so diagnostic records are sent to a single socket instead of broadcasting to all open sockets.
  - `DiagnosticTransportOrder` now parses incoming diagnostic orders (IDs 1555, 2555, 3000) and re-publishes them via `Hub::publish_runtime_*()`, enabling forwarding of remote board diagnostics to the control station.
  - `Diagnostics::Hub::flush_pending` throttles records to at most one per 100 ms per priority level (urgent and non-urgent tracked separately) to prevent flooding sinks.
  - Default TCP keepalive values tightened: 30 ms idle, 20 ms interval, 2 probes (commented out for now since it can't keep connection with the backend).
  - `lwipopts.h`: `MEMP_NUM_SYS_TIMEOUT` increased by 10 to avoid pool exhaustion with SNTP and whatever else it may happen, `TCP_TMR_INTERVAL` reduced to 10 ms for faster keepalive response.
  - Added `Diagnostics::install_ethernet_sink(OrderProtocol*)` to register the Ethernet sink with a specific target socket.
- Makes the watchdog reset check block the system and have a visual indication similar to hardfault

### Fixes

- Removes the default SNTP server from Ethernet configuration, defaulting to nullptr instead
- Fix a warning about symbols in HardfaultTrace (doesn't change any behaviour)
- Adds a check to ensure return values from Scheduler::get_global_tick() are monotonic (never get decreasing values)

## v6.1.3 - 2026-06-02

### Fixes

- Drive SPI4 and SPI5 from HSI for stable 2 MHz BMS transfers
  SPI4 and SPI5 now use the 64 MHz HSI peripheral clock source instead of PCLK2,
  allowing a 2 MHz request to select the /32 SPI prescaler deterministically.

## v6.1.2 - 2026-05-29

### Fixes

- Fix faults caused in state machine start causing non-initialized state machine
- Fix some clock problems in spi 4 and 5

## v6.1.1 - 2026-05-11

### Fixes

- Allow protections to read volatile sample sources
  Protection sources now preserve the storage cv-qualification through a reference source wrapper,
  so protections can be declared directly over volatile variables while rule evaluation still uses
  plain sample values.

## v6.1.0 - 2026-05-11

### Features

- It is just a one word change, makes it possible to use `Board::instanceof()` to get a `constexpr auto& obj`. It works perfectly as long as you don't try using the reference before `Board::init()`.

### Fixes

- Fix reorder issue in PWM constructor
- Unify get_global_tick behaviour and make format_numeric_value return if value was written in DiagnosticFormatter
- The macros changes in fix/volatiles where reverted in a merge in the last release, this pr applies the changes once more to fix the undefined behaviour.

## v6.0.0 - 2026-05-06

### Breaking Changes

- Redesign fault handling, protections, and Board bootstrap around explicit fault policies
  This PR changes the public integration contract for applications built on ST-LIB.

  Breaking changes:

  - `Board` now takes the fault policy type as its first template parameter.
  - The global `FAULT` runtime is owned exclusively by `FaultController`.
  - User state machines are now nested under the global `OPERATIONAL` state through `FaultPolicy` or `FaultPolicyNoMachine`.
  - Protections are now compile-time `Board` request objects evaluated through `Board::ProtectionEngine`; the previous `ProtectionManager` and boundary split is no longer the active model.
  - Runtime reporting is unified under `PANIC(...)`, `FAULT(...)`, `WARNING(...)`, and `INFO(...)`.
  - The real bootstrap path is `Board::init()`. Legacy `STLIB::start()`, `STLIB::update()`, `STLIB_LOW::start()`, and `STLIB_HIGH::start()` must not be used as the integration path.

  Migration notes:

  - Declare the board as `Board<YourFaultPolicy, ...>`.
  - Use `FaultPolicy<app_machine, on_fault_enter>` when you want an operational state machine nested under the global runtime.
  - Use `FaultPolicyNoMachine<on_fault_enter>` when you only need a fault-entry callback.
  - Use `DefaultFaultPolicy` when you want neither an operational machine nor a fault-entry callback.
  - Declare protections with `Protections::protection<"name", source>(...)` and pass the resulting request objects to `Board`.
  - In the main loop, drive the runtime through `FaultController::check_transitions()`, `Board::evaluate_protections()`, and `Diagnostics::Hub::flush()`.

### Features

- add an ethernet connected check
- move initialization outside constructor in PWM, DualPWM, Encoder and InputCapture
- Small refactor of some spi and timerwrapper functionality
  timerwrapper:
   - Add `set_callback(void (*callback)(void*), void* callback_data)` to set the callback and its data instead of needing to call `configurexxbit()` and set the period.
   - `set_limit_value(uint32_t arr)` to set the arr, this will likely be changed to use a `uint32_t` type only when using a 32 bit timer, for now it is just an alias to `instance->tim->ARR = arr;`. Was not added in this pr but also wasn't mentioned before.

  spi:
   - Add `transceive` with ptr + data explicitly instead of using a span since it's sometimes a pain in the ass to use.

### Fixes

- Fix incorrect usage of non-volatile buffers in peripherals. This wrong usage was causing undefined behaviour, since the compiler could optimize out reads and writes that should have been going directly to memory.
  There's a minor API change: now the MPU macros for memory regions include the volatile keyword for the non-cached variants.
- MPUDomain dynamic regions were configured using the linker symbol values instead of their addresses (that is the correct way of using linker symbols). That made non-cached regions cached, and caused harware undefined behaviour.
- fix pwm channel 4 config in TimerWrapper
- fix prescaler calculation in scheduler

## v5.2.0 - 2026-04-22

### Features

- move a struct outside template class
- Create a parameter to choose the type of the variables

### Fixes

- RXER pin for Ethernet is now optional
  H11 ethernet doesn't use RXER pin. The RXER pin is now a pointer so it can hold a nullptr. Ethernet constructor now has a std::optional for this pin
- input capture hotfix for errorhandler
  fix: make dummy have invalid channel nº for rising, falling

## v5.1.0 - 2026-04-05

### Features

- Refactor the ADC stack around DMA-backed acquisition using new MPU
  Includes the DMA-backed `ADCDomain` migration, the shared ADC sensor base, and the updated ADC integration/tests in the `adc-dma` branch.
- Added module dfsdm tested
- fix remaining scheduler race conditions and add warning when tasks are not ran in time
  The mechanism for checking if tasks are not ran in time is very simple but that also means the scheduler can only know if a task has not been called when the waiting time runs out for the second time.
  This means you will know if you're too slow to execute the task in less than 2x its period but not if you're between 1 and 2x its period.
- input capture implementation
  Also changed how the timer interrupts are handled since this was required for inputcapture

### Fixes

- add a FLASH_ST in .ld to keep the flash information that is not code
- Fix incorrect GPIO alternate-function availability bitmaps
  Correct the STM32H723 pin alternate-function masks in `Pin.hpp` so GPIO
  configuration reflects the actual AF support per pin.

### Internal

- Introduce semver tooling and release automation infrastructure
  Bootstrap the semiautomated release flow for ST-LIB without publishing a new
  library version yet.
