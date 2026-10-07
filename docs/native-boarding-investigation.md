# Native boarding investigation — initial trace

Task 004 is active. These are static findings for executable SHA-256
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
Addresses are preferred virtual addresses with image base `0x140000000`.
None is a runtime-validated patch site.

## Located functions

RIP-relative references to diagnostic strings were matched to PE exception-table
function ranges, then disassembled/decompiled using Ghidra 12.1.3 and
`tools/ghidra/ExportFunctions.java`. Raw outputs are ignored under
`analysis/native-boarding/`.

| Function entry | Evidence / role |
| --- | --- |
| `0x140b0db00` | Assertions name `ecs::SimEntityAtTerminalSystem::Update`; range ends `0x140b0fa9c` |
| `0x140b32d00` | Assertion names `NoteSimEntityNeedsPathChanged`; range ends `0x140b32fc7` |
| `0x140b32fd0` | Assertion names `NoteSimPersonDoneWaiting`; range ends `0x140b332f3` |
| `0x140b89990` | Assertions name `TransportVehicleSystem::HandleVehicleArrived`; range ends `0x140b8a1b8` |
| `0x140b86510` | Two-part key lookup used by terminal update; returns stored collection or empty fallback |
| `0x140b86440` | Assertion names `TransportVehicleSystem::GetLineCargoInfo` |
| `0x140b35b60` | Assertion names `SimPersonDrivingArriveAtEndOfPath`, called by done-waiting |

## What this establishes

The terminal update calls `0x140b86510` at `0x140b0e357` with two integers
from the current queue entry. It also accesses a line stop at a `0x90` stride
and checks cargo loading data, including `GetLineCargoInfo`. This is consistent
with line/stop-specific vehicle selection. The helper itself constructs an
eight-byte key and queries a cached map. Its exact exported binding and the
complete eligibility/selection loop still need tracing before claiming the
precise cause of interlining failure.

The update batches changes and performs component-update notifications later
in the function. A diagnostic assertion references `LostNoLineVehicleId`.
These are further reasons to investigate engine-owned transitions instead of
overwriting a passenger's visible line field. Component identities and all
associated queue/manifest changes are not yet established.

The arrival handler updates vehicle timing/loading fields before invoking a
signal near its end. We have not yet proved this signal's full relationship to
the Lua arrival event or its ordering relative to terminal queue processing.

`NoteSimPersonDoneWaiting` calls `0x140b35b60`, whose diagnostic identifies
private-driving end-of-path processing. Do not treat the generic phrase “done
waiting” as a supported transit boarding interception point.

## Next steps and reproducibility

Inspect `0x140eecdc0`, an API-side caller of the two-key lookup, to verify its
binding; trace collection consumers inside terminal update through selection
and component commits. Validate any inferred transition with runtime evidence.
Do not infer thread safety from a log thread name or decompiler types.

Example against the existing Ghidra project:

```sh
rtk proxy env GHIDRA_HEADLESS_MAXMEM=4G python3 tools/ghidra_headless.py \
  analysis/ghidra TPF3Interlining -process TransportFever3.exe -noanalysis \
  -scriptPath tools/ghidra \
  -postScript ExportFunctions.java analysis/native-boarding/entry-functions.txt \
  0x140b0db00 0x140b32d00 0x140b32fd0 0x140b89990 \
  -log analysis/native-boarding/targeted.log
```

The [revision 2 probe](../probes/pathfinding/README.md) complements this trace
with read-only line-name, terminal, capacity, and waiting-state snapshots.
Its new queries are based on shipped declarations and bundled GUI usage:
`SimEntityAtTerminalSystem.getLineStopSimEntities`, `LineSystem.getLineStops`,
`SimEntityAtVehicleSystem.getVehicleSimEntitiesCountForCargoType`, and
`cargoTypeRep.getPassengerCargoTypeId`. The game uses zero-based stop/cargo IDs
and one-based Lua arrays, corroborated by the station-group and vehicle GUI code.

Revision 2's new queries passed the runtime check in
[Test 2](test-run-2026-10-07-2.md). Use its S1/S3 Wimborne-to-Melksham case
for the first same-terminal intervention; S2 uses a separate terminal.
No engine hook, native extension, or boarding behavior change is implemented.
