# Protections, Faults, and Diagnostics

This document describes the current protection, fault, diagnostic, and transmission model in
`ST-LIB`.

It is intentionally split into two parts:

- **How to use**
- **Internal development**

If you only want to integrate protections into an application, read the first part only.

## 1. How to Use

### 1.1 Mental Model

The subsystem has three explicit runtime operations:

- `Board::init()`
- `Board::ProtectionEngine::evaluate()` or `Board::evaluate_protections()`
- `Diagnostics::Hub::flush()`

If the application uses an operational state machine nested under the global runtime, it also
polls:

- `FaultController::check_transitions()`

The global fault model is always the same:

- the framework owns a global runtime with two states: `OPERATIONAL` and `FAULT`
- internally, the only way to enter the global `FAULT` state is `FaultController::request_fault(...)`
- protection faults, `PANIC(...)`, and `FAULT(...)` all end up there
- fault diagnostics are transmitted with urgent priority through `Diagnostics`

```mermaid
flowchart TD
    A["Declare protection rules"] --> B["Declare Board policy and request objects"]
    B --> C["Board::init()"]
    C --> D["while (1)"]
    D --> E["FaultController::check_transitions()"]
    D --> F["Board::evaluate_protections()"]
    D --> G["Diagnostics::Hub::flush()"]
```

The application integration contract is:

```cpp
Board<FaultPolicyT, dev0, dev1, ...>
```

Where:

- `FaultPolicyT` is mandatory and is always the first template argument
- `dev0, dev1, ...` are board declarations, including hardware requests and protection requests
- the framework always owns the top-level runtime machine
- the application may optionally provide a nested operational machine and/or a `FAULT` entry callback

### 1.2 Declaring Protections

Protections are compile-time board requests. A protection request:

- has a stable name encoded in the type
- reads from one sample source object or sample variable
- owns a fixed set of rules declared before runtime
- is passed to `Board<...>` with the rest of the board request objects

Rules are created through the factories in `Protections::Rules` and passed to
`Protections::protection<id, source>(...)`.

Available rule factories:

- `Rules::below(...)`
- `Rules::above(...)`
- `Rules::range(...)`
- `Rules::equals(...)`
- `Rules::not_equals(...)`
- `Rules::time_accumulation(...)`

Rule factories return `std::expected`; `Protections::protection(...)` unwraps them while building
the compile-time declaration. Invalid declarations fail during build or constant evaluation instead
of creating a partial runtime registry.

Current rule signatures are:

```cpp
Rules::below(fault_threshold)
Rules::below(fault_threshold, warning_threshold)

Rules::above(fault_threshold)
Rules::above(fault_threshold, warning_threshold)

Rules::range(low_fault, high_fault)
Rules::range(low_fault, high_fault, low_warning, high_warning)

Rules::equals(value)
Rules::not_equals(value)

Rules::time_accumulation(fault_threshold, window_seconds)
Rules::time_accumulation(fault_threshold, warning_threshold, window_seconds)
```

`Rules::time_accumulation(...)` has these semantics:

- it is intended for floating-point samples
- it evaluates `abs(sample)`
- it measures continuous active time, not an integral over samples
- it resets the accumulated active time when the triggering condition clears
- it uses `Scheduler::get_global_tick()`, so it does not depend on the `while (1)` iteration rate

### 1.3 Protection Lifecycle

Declare protections at namespace scope and pass them to `Board`.

The intended lifecycle is:

1. compile-time declaration
2. `Board::init()`
3. evaluation and flushing in the runtime loop

There is no runtime registration phase and no mutable protection registry. `Board` derives a
board-specific `ProtectionEngine` type from the protection requests it receives, initializes it from
`Board::init()`, and then starts the global fault runtime.

### 1.4 Typical Protection Example

```cpp
#include "ST-LIB.hpp"

using namespace ST_LIB;

constexpr auto led = DigitalOutputDomain::DigitalOutput(PF13);

constexpr uint16_t bus_voltage_id = 1001;
inline constexpr auto bus_voltage_protection = Protections::protection<bus_voltage_id, bus_voltage>(
    Protections::Rules::below(350.0f, 370.0f),
    Protections::Rules::time_accumulation(20.0f, 15.0f, 0.5f)
);

using MainBoard = Board<DefaultFaultPolicy, bus_voltage_protection, led>;

int main() {
    MainBoard::init();

    while (1) {
        MainBoard::evaluate_protections();
        Diagnostics::Hub::flush();
    }
}
```

### 1.5 Global Fault Runtime

`Board::init()` always installs and starts the global fault runtime.

That runtime has two states:

- `OPERATIONAL`
- `FAULT`

If the application does not use a functional state machine, nothing else is required.

Typical choices are:

- `Board<DefaultFaultPolicy, ...>` when no extra fault callback is needed
- `Board<FaultPolicyNoMachine<on_fault_enter>, ...>` when only `FAULT` entry actions are needed
- `Board<FaultPolicy<app_machine, on_fault_enter>, ...>` when both a nested operational machine and `FAULT` entry actions are needed

If the application does use a functional state machine, it can be nested inside `OPERATIONAL`
through a `FaultPolicy`.

Example:

```cpp
enum class AppState : uint8_t { IDLE = 0, RUN = 1 };

static constexpr auto idle_state = make_state(AppState::IDLE);
static constexpr auto run_state = make_state(AppState::RUN);

static inline auto app_machine = make_state_machine(AppState::IDLE, idle_state, run_state);

static void on_fault_enter() {
    // disable power stage, set LEDs, open contactors, etc.
}

using MainBoard = Board<FaultPolicy<app_machine, on_fault_enter>, led>;

int main() {
    MainBoard::init();

    while (1) {
        FaultController::check_transitions();
        MainBoard::evaluate_protections();
        Diagnostics::Hub::flush();
    }
}
```

Important rules:

- the user state machine models operational behavior only
- the user does not program transitions to the global `FAULT`
- if a fatal condition must force the system into `FAULT`, user code should use `PANIC(...)` or
  `FAULT(...)`
- if a nested operational state machine is used, poll `FaultController::check_transitions()`, not
  the child machine directly
- `Board` takes the fault policy type as its first template argument

`on_fault_enter` semantics:

- it is an optional callback owned by the global fault runtime
- it runs when the global runtime enters `FAULT`
- it is the right place to perform application fault-entry actions such as disabling power stages,
  opening contactors, or setting status LEDs
- it does not replace the fault transition itself; it is an enter action attached to the global
  `FAULT` state

If the application needs neither a nested machine nor a `FAULT` entry action, use:

```cpp
using MainBoard = Board<DefaultFaultPolicy, led>;
```

### 1.6 Runtime Diagnostics API

The runtime diagnostic façade is:

- `PANIC(...)`
- `FAULT(...)`
- `WARNING(...)`
- `INFO(...)`

Their semantics are:

- `PANIC(...)`: fatal runtime/internal error, enters the global `FAULT`
- `FAULT(...)`: fatal domain/application fault, enters the global `FAULT`
- `WARNING(...)`: non-fatal diagnostic
- `INFO(...)`: informational diagnostic

`PANIC(...)` and `FAULT(...)` both call the same global fault path underneath.
The difference is semantic classification of the cause and diagnostic category.

### 1.7 Internal Fault Primitive

Internally, protections and fatal runtime reporters converge on:

```cpp
FaultController::request_fault(cause);
```

This primitive is not part of the normal user-facing API.
In the current implementation it is an internal `FaultController` entry point, not a public
application hook.

User code should prefer `FAULT(...)` or `PANIC(...)` so the library captures consistent source
metadata and preserves the public runtime contract.

In practice:

- protections use `FaultController::request_fault(...)` internally
- `PANIC(...)` and `FAULT(...)` use that same path internally
- user application code should not call `request_fault(...)` directly

### 1.8 Transmission Semantics

All external reporting goes through `Diagnostics`.

There is no separate fault-broadcast subsystem anymore.

The transmission model is:

- normal diagnostics are queued with `NORMAL` priority
- faults are published with `URGENT` priority
- `Diagnostics::Hub::flush()` always drains urgent records first

Default sinks are installed during `Board::init()`:

- UART sink when UART printing is available
- TCP sink when `STLIB_ETH` is enabled

If a transport is not compiled in, it is simply not installed.

### 1.9 Migration From the Legacy Model

If you are migrating from the previous architecture:

- stop using `ProtectionManager`
- stop using the low/high protection split
- stop using `Boundary` / `BoundaryInterface` as the protection integration model
- stop depending on `FaultRuntime`
- stop treating `STLIB::start()`, `STLIB::update()`, `STLIB_LOW::start()`, or `STLIB_HIGH::start()`
  as the real bootstrap path
- move bootstrap to `Board::init()`
- declare `Board<fault_policy, ...>` explicitly
- move operational user behavior into `FaultPolicy<app_machine, on_fault_enter>` when needed
- stop programming transitions to the global `FAULT`
- replace legacy reporting paths with `PANIC(...)`, `FAULT(...)`, `WARNING(...)`, and `INFO(...)`

## 2. Internal Development

### 2.1 Architectural Overview

The design is split into four concerns:

- **protections**: evaluate domain rules over samples
- **faulting**: control the global `OPERATIONAL/FAULT` runtime
- **diagnostics**: store and dispatch structured records
- **transport**: serialize and emit diagnostics

```mermaid
flowchart LR
    A["ProtectionEngine"] --> B["FaultController::request_fault(...)"]
    A --> C["Diagnostics::Hub"]
    D["PANIC / FAULT"] --> B
    E["WARNING / INFO"] --> C
    B --> F["FaultCause"]
    F --> G["FaultDiagnosticMapper"]
    G --> C
    C --> H["DiagnosticSink"]
    H --> I["UART / TCP"]
```

The key boundaries are:

- protections do not know transport
- diagnostics do not own or evaluate protections
- `FaultController` does not own sinks
- transport does not change system state

### 2.2 Protection Domain Model

Public API:

- `Protections::protection<id, source>(...)`
- `Board::ProtectionEngine`
- `Board::evaluate_protections()`
- `Protections::Rules::*`

Internal model:

- one board-specific compile-time collection of protections
- no low/high frequency split in the domain model
- rule configuration returned through `std::expected`
- rule evaluation produces `RuleState`, `RuleEdge`, and `RuleSnapshot`

Supported rule kinds:

- `BELOW`
- `ABOVE`
- `RANGE`
- `EQUALS`
- `NOT_EQUALS`
- `TIME_ACCUMULATION`

`TIME_ACCUMULATION` uses `Scheduler::get_global_tick()` to measure real elapsed time.
It no longer assumes a fixed evaluation rate.

`Board::ProtectionEngine::evaluate()`:

- walks every protection
- publishes non-fatal rule edges through `Diagnostics`
- requests the global fault when a rule reaches `FAULT`
- throttles repeated fault notifications with `notify_delay_in_microseconds`

### 2.3 Global Fault Runtime

`FaultController` owns the global runtime state machine.

The runtime machine is:

- always present
- always two-state: `OPERATIONAL` / `FAULT`
- optionally composed with a nested operational machine through `FaultPolicy`

Responsibilities of `FaultController`:

- own and start the global runtime
- latch the first `FaultCause`
- request transition to the global `FAULT`
- execute the user `on_fault_enter` callback through the `FAULT` state enter action
- publish the fault diagnostic with urgent priority

Important invariant:

- entering the global `FAULT` never depends on transport delivery succeeding

### 2.4 Early-Fault Bootstrap Semantics

The fault path is valid during `Board::init()`.

That is why `Board::init()` installs, as early as possible in the bootstrap path:

- default diagnostic sinks
- the global fault runtime

before clock/peripheral setup and before subsystem initialization that may trigger `PANIC(...)`.

If a fatal request arrives before the global runtime has been started:

- the cause is latched
- the runtime is rebuilt so that it starts directly in `FAULT`
- the urgent fault diagnostic is still published through `Diagnostics`

If the diagnostic record is produced before any sink exists, it is still retained in local history.
When the first sink is installed, the retained history is replayed into the pending queue so the
record can still be delivered later.

This avoids losing early boot faults and other pre-transport diagnostics.

### 2.5 FaultCause and Diagnostic Mapping

`FaultCause` is not a `DiagnosticRecord`.

`FaultCause` is the control-plane object for fatal conditions.
It stores:

- fault kind
- stable origin
- runtime fault payload or protection fault payload

`Diagnostics::DiagnosticRecord` is the reporting-plane object.

Conversion between both is explicit:

- `FaultController` latches and operates on `FaultCause`
- `FaultDiagnosticMapper` converts `FaultCause` into a `DiagnosticRecord`
- `Diagnostics::Hub` only stores and delivers `DiagnosticRecord`

This keeps the global fault runtime independent from the storage and transport shape of diagnostics.

### 2.6 Diagnostics Model

`Diagnostics::Hub` is a fixed-capacity internal event bus.

Main types:

- `DiagnosticRecord`
- `RuntimeDiagnosticPayload`
- `ProtectionDiagnosticPayload`
- `DiagnosticSink`

Supporting components:

- `RecordFactory`
- `DiagnosticFormatter`
- `DiagnosticTimestampProvider`

Memory policy:

- fixed sink storage
- fixed history ring
- fixed pending queue
- no heap in `publish()`
- no heap in `flush()`

Priority policy:

- `NORMAL`
- `URGENT`

`flush_urgent()` drains urgent records only.
`flush()` drains urgent first and then normal records.

### 2.7 Runtime Reporters

The runtime reporters are intentionally thin façades.

`PANIC(...)`, `FAULT(...)`, `WARNING(...)`, and `INFO(...)`:

- capture source metadata with `std::source_location`
- format the runtime message into a fixed stack buffer
- publish a diagnostic or request a fault

They do not use shared mutable metadata anymore.
That keeps the reporting path reentrant and removes the old `SetMetadata + Trigger` split.

### 2.8 Timestamp Semantics

`DiagnosticTimestampProvider` does not start RTC services from the diagnostic hot path.

If RTC is already running and has valid time, records use RTC timestamp data.
Otherwise, diagnostics fall back to uptime when available.

This avoids recursive or bootstrap-dependent fatal paths while timestamping diagnostics.

### 2.9 C++23 Design Choices

This subsystem uses a narrow set of C++23 features where they provide direct value:

- `std::expected`
  for explicit rule configuration failure
- `std::variant` and `std::visit`
  for static rule composition
- `concepts`
  to constrain rule factories and sample sources
- `std::source_location`
  for runtime reporter metadata without mutable globals
- `std::to_underlying`
  for transport encoding
- `std::byteswap` and `std::endian`
  in the TCP diagnostic encoder
- `std::span<std::byte>`
  for fixed binary transport encoding

The intent is not to maximize feature usage.
The intent is to improve:

- correctness
- API clarity
- determinism
- suitability for embedded firmware

### 2.10 Firmware Invariants

The subsystem is expected to preserve these invariants:

- no heap in protection evaluation
- no heap in diagnostic publish/flush
- no shared mutable metadata in runtime reporters
- no transport logic inside protection rules
- no separate fault-broadcast path outside `Diagnostics`
- fixed-capacity storage for protections, sinks, history, and pending queue
- explicit lifecycle: register, init, evaluate, flush
