# Normal mod loading and native Linux — 2026-10-13

## Release requirement

Interlining should be installed through mod.io and enabled normally. A shared
script-hook dependency is acceptable; an external loader is not. The current
Windows DLL plus `bridge_loader.exe` is a research implementation and cannot be
published as a working ordinary Lua mod merely by repackaging it.

## Confirmed local evidence

The native Linux installation supplied by the user contains an x86-64 ELF PIE,
126,427,434 bytes, SHA-256
`ad2e3b7bc26404323d952c21f1d9a2d36d0c4b6037cf5dc831e88811a7e00af4`.
The public command, engine-component, main-script and mod declarations are
byte-identical to the inspected Windows installation. Mod dependencies and load
ordering are declared; no native-library entry point is declared in the mod type.
The documented command surface still lacks a passenger waiting-leg replacement.
The debug component exchange declaration is restricted to emission/model components.

Targeted Windows decompilation: state setup at `0x142faff70` either removes the
whole `package` global or clears `loaders`, `loadlib`, `path`, `cpath`, `preload`
and `seeall` fields. Helper `0x142fbe3a0` pushes nil; `0x142fbe8f0` assigns a field.
These are static interpretations checked against helper bodies, not exported names.
Linux disassembly shows the corresponding nil/field operations at
`0x4633459`–`0x463346d` for `loadlib` and `0x4633491`–`0x46334a5` for `cpath`;
`0x460d690` implements the matching nil push. ELF addresses are relative to the
PIE image, not live process addresses. No executable was modified or injected.

This rules out assuming `package.loadlib` is available. It does not prove every
possible module-loading path absent: remaining searchers and actual mod runtime
contexts need inspection. Do not treat the presence of library strings in an
executable as proof that mods can call that library.

Generated evidence: `analysis/hook-investigation/lua-module-loading.txt`,
`lua-module-primitives.txt`, `linux-lua-module-references.json`. Windows targeted
exports use `tools/ghidra_headless.py` and `tools/ghidra/ExportFunctions.java`.
Linux instruction sequences were inspected using bounded `objdump -d` ranges.

## Runtime check prepared

[The capability probe](../probes/runtime/README.md) is a normal Lua mod, installed
under the native Linux game's `mods/tpf3_interlining_runtime_probe_1/`. It logs
function/table availability in pre-run, run, post-run and simulation-update
contexts, without calling module loaders or mutation commands. Isolated Lua
checks confirm missing globals are handled, output is once per phase and a
present loader is inspected without invocation. Five runtime files verified
against the repository; package under ignored `analysis/packages/`.

User agreed to launch native Linux and enable it on a disposable save. Actual
runtime evidence is pending. No native backend was loaded and no functional
interlining mod was uploaded. A Linux backend would also need platform-specific
engine layouts, calls, memory access and hook handling; the Windows offsets and
calling convention cannot simply be reused.

## Discovery finding retained

0.14.0 indexes only one newly scanned passenger per discovery callback and skips
discovery while pending boardings occupy the budget. At3,000 callbacks the saved
run had216 indexed people across25 stations, while647 of781 screens used global
fallback. Its pending peak was3 and capacity holds0. This supports improving
index coverage and station-focused work, not simply increasing pending slots.
Lua already exposes station line-stop and waiting-queue queries; a script-based
discovery layer could use those directly if a viable mutation capability exists.

Next: inspect runtime loading capabilities, then choose the supported release
architecture. If the required hook is unavailable, document the precise missing
engine API rather than promise a script dependency can manufacture it.

## Public references

The [official scripting guide](https://wiki.transportfever3.com/doku.php?id=modding:general:modscripts)
describes Lua/Teal entry scripts and resource callbacks, not native plugin loading.
The [official API index](https://wiki.transportfever3.com/script-doc/) identifies
engine access as reading state and separates commands. Publishing/guideline pages
returned HTTP429 during this check; native-binary acceptance on mod.io has not
been established. Do not infer rejection or acceptance from that fetch failure.
