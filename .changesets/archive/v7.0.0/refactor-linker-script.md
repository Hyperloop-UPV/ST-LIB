release: major
summary: Refactor linker script, startup code, and memory model with unified copy/zero tables, configurable ITCM, and weak BoardInit. Also fixes LTO.

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
