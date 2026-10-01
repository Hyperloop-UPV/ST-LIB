# ST-LIB Board Contract

This document defines the contract of [`Inc/ST-LIB.hpp`](../Inc/ST-LIB.hpp), especially the
`ST_LIB::BuildCtx` and `ST_LIB::Board` machinery.

If you change a domain, add a new domain, or add a cross-domain composition rule, read this first.

## 1. Mental Model

`Board<Policy, Tree, ...>` is a compile-time build pipeline plus a runtime init pipeline.

- Compile time decides what exists and how it must be configured.
- Runtime only materializes already-built configurations and links HAL handles.

`Board` is intentionally declarative. Request objects describe intent; domains convert that intent
into concrete configs. The first template argument is the global fault runtime policy type used by `FaultController`.  The second is a precomputed `ClockDomain::ClockTree`.

Protection declarations are also request objects. They do not inscribe hardware into `BuildCtx`;
instead, `Board` filters them into a board-specific `ProtectionEngine` type.

## 1.1 Board Policy Contract

The first template argument of `Board` must be a fault policy type.

That type must expose:

- `static constexpr bool has_operational_machine`
- `static constexpr Callback on_fault_enter`
- `static constexpr auto& operational_machine` when `has_operational_machine == true`

`FaultPolicy<...>`, `FaultPolicyNoMachine<...>`, and `DefaultFaultPolicy` are the intended public
helpers for this contract.

## 1.2 Board Clock Tree

The second template argument of `Board` must be a `ClockDomain::ClockTree`.

```cpp
template <BoardFaultPolicy FaultPolicyT, auto& ClockTreeDef, auto&... devs> struct Board { ... }
```

- Use `ST_LIB::default_clock_tree` (hardcoded for 8 MHz or 25 MHz HSE, 550 MHz SYSCLK) if no host-tool-generated tree is available.
- `Board::build()` passes the tree to `ClockDomain::build()` for compile-time validation.
- `Board::init()` passes the tree to `ClockDomain::Init::init()` for runtime HAL configuration.
- Domains that depend on clock frequencies (Timer, SPI, ADC, SDMMC) receive the tree in their own
  `Init::init()`.

See [`clockdomain.md`](clockdomain.md) for the full ClockDomain contract.

## 2. Domain Contract

Every domain used by `BuildCtx` and `Board` must provide:

- `static constexpr std::size_t max_instances`
- `struct Entry`
- `struct Config`
- `template <size_t N> static consteval std::array<Config, N> build(std::span<const Entry>)`
- `template <std::size_t N> struct Init`

`Init<N>` must provide:

- `static inline std::array<Instance, N> instances`
- `static void init(std::span<const Config, N> ...)`

The exact runtime dependencies of `init(...)` are domain-specific, but they must be explicit in the
signature.

## 3. Request Object Contract

A request object that can be used after the first `Policy` argument inside `Board<Policy, ...>` must
provide:

- `template <class Ctx> consteval ... inscribe(Ctx&) const`

or any compatible return type if it naturally inscribes multiple dependent entries.

`inscribe(ctx)` must:

- be `consteval`
- append the domain entries needed by the request
- return indices only for later compile-time wiring
- never depend on runtime state

Hardware requests also expose `using domain = <DomainType>;` for `Board::instance_of<request>()`.
Protection requests intentionally do not expose a hardware domain and are accessed through
`Board::ProtectionEngine`.

## 4. BuildCtx Contract

`BuildCtx` is append-only.

Important consequences:

- `BuildCtx::add(...)` does not deduplicate.
- If two requests emit the same physical resource twice, `BuildCtx` will keep both.
- Preventing duplicates is the responsibility of request objects or the destination domain build
  logic.

This is a deliberate design choice. `BuildCtx` is a storage and ownership map, not a resolver.

## 5. Board Build Contract

`Board::build_ctx()`:

- creates a `DomainsCtx`
- evaluates every request object's `inscribe(ctx)` in declaration order
- does not inspect or route the `Policy` through `BuildCtx`

`Board::build()`:

- computes domain sizes
- runs each domain `build<N>(...)`
- assembles a `ConfigBundle`

`Board::cfg` is the compile-time result of that process.

`Board::ProtectionEngine` is the compile-time protection engine assembled from the protection
request objects in the same `Board<Policy, ...>` declaration.

No runtime-only configuration logic belongs here.

## 6. Board Init Contract

`Board::init()`:

- must only consume `cfg`
- must initialize domains in dependency order
- must not invent new hardware resources
- must not re-resolve compile-time relationships

Permitted runtime work:

- HAL init
- clock enable
- IRQ enable
- handle linking
- initializing the board-specific protection engine
- starting the global fault runtime
- buffer allocation if the buffer is inherently runtime memory
- starting peripherals using already-built configs

Forbidden runtime work:

- choosing a DMA request dynamically
- choosing a stream dynamically
- changing a domain topology
- creating extra domain entries not represented in `cfg`

## 7. Owner Mapping Contract

`Board::instance_of<request>()` relies on owner pointers captured during `BuildCtx::add(...)`.

Therefore:

- each stored `Entry` must be associated with the request object that owns it
- owner identity must remain stable across compilation
- `instance_of` is only valid for request objects actually inscribed into `Board`

## 8. DMA Composition Contract

There are two valid ways for a domain to use `DMADomain`.

### 8.1 Direct inscription

Use this when one request object maps directly to one or more DMA resources.

Example: SPI.

The request object may hold `DMADomain::DMA<...>` and inscribe it directly.

### 8.2 Compile-time DMA contributions

Use this when multiple request objects share one physical DMA resource and direct inscription would
duplicate it.

Example: ADC, where many channel requests can belong to one ADC peripheral and one DMA stream.

In this case, the domain must expose compile-time contribution helpers:

- `*_contribution_count(...)`
- `build_*_contributions<...>(...)`

Those helpers must:

- be `consteval`
- be additive only
- preserve all pre-existing `DMADomain` entries
- synthesize only the missing DMA entries for that domain

`Board` may then merge base DMA entries with these contributions through generic helpers such as
`BuildUtils::merge_dma_entries(...)` and `BuildUtils::build_dma_configs(...)`.

## 9. ADC-Specific Rule

`ADCDomain` currently uses the DMA contribution pattern.

Reason:

- request granularity is one ADC channel request
- physical DMA granularity is one DMA stream per ADC peripheral

If ADC ever changes to a request type that directly represents a full ADC peripheral, it could move
to the direct inscription pattern used by SPI.

## 10. How To Add a New DMA-Using Domain

If the domain has a 1:1 request-to-DMA mapping:

1. Inscribe `DMADomain::DMA<...>` directly from the request object.
2. Store DMA indices in the domain `Entry`.
3. Use those indices during domain `build/init`.

If the domain has a many:1 request-to-DMA mapping:

1. Keep the domain request objects focused on their logical resource.
2. Build the domain configs first.
3. Expose compile-time DMA contribution helpers for the missing shared DMA entries.
4. Merge those contributions into the base `DMADomain` entries in `Board::build()`.

## 11. Non-Negotiable Invariants

- Topology is compile-time.
- `BuildCtx` is append-only and non-deduplicating.
- `cfg` is the full source of truth for runtime init.
- Runtime init may materialize resources, but not invent topology.
- Cross-domain composition must be explicit and compile-time.

## 12. Practical Review Checklist

When reviewing a change to `Board`, a domain, or a DMA-using peripheral, verify:

- Does the request object inscribe only compile-time information?
- Can duplicate physical resources be produced? If yes, where are they prevented?
- Does runtime init consume only `cfg`?
- Are protection requests passed through the board and evaluated through `Board::ProtectionEngine`
  or `Board::evaluate_protections()`?
- Is any stream/request/allocation decision being made too late?
- If the domain uses shared DMA, is it contributing only missing entries and preserving the rest?
