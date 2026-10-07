# Native boarding investigation

Task 004 is active. These are static findings for executable SHA-256
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
Addresses are preferred virtual addresses with image base `0x140000000`.
None is a runtime-validated patch site.

## Located functions

RIP-relative references to diagnostic strings were matched to PE exception-table
regions, then disassembled/decompiled using Ghidra 12.1.3 and
`tools/ghidra/ExportFunctions.java`. Raw outputs are ignored under
`analysis/native-boarding/`.

`tools/native_references.py` reproduces candidate searches but is not a
disassembler. Exception regions can describe cold assertion fragments:
references near `0x140b0c7c9` and `0x140b0c88e` belong to functions beginning
at `0x140b0c730` and `0x140b0c7f0`. Confirm boundaries in disassembly.

| Function entry | Evidence / role |
| --- | --- |
| `0x140b0db00` | Assertions name `ecs::SimEntityAtTerminalSystem::Update`; range ends `0x140b0fa9c` |
| `0x140b32d00` | Assertion names `NoteSimEntityNeedsPathChanged`; range ends `0x140b32fc7` |
| `0x140b32fd0` | Assertion names `NoteSimPersonDoneWaiting`; range ends `0x140b332f3` |
| `0x140b89990` | Assertions name `TransportVehicleSystem::HandleVehicleArrived`; range ends `0x140b8a1b8` |
| `0x140b86510` | Two-part key lookup used by terminal update; returns stored collection or empty fallback |
| `0x140b86440` | Assertion names `TransportVehicleSystem::GetLineCargoInfo` |
| `0x140b35b60` | Assertion names `SimPersonDrivingArriveAtEndOfPath`, called by done-waiting |
| `0x140b0c730`, `0x140b0c7f0` | Terminal component pre/post-change callbacks; vehicle reverse index |
| `0x140b0c8b0`, `0x140b0d030` | Terminal entity-added/removal callbacks; line/stop queues |
| `0x140b285d0` | Lambda in `NoteSimEntityAtTerminalChanged`; handles assigned vehicle |
| `0x140b33a30` | `NoteWalkPersonsArrived`; completes approach to vehicle |
| `0x140b364a0` | Person-state cleanup, including terminal components |
| `0x140b38950` | Vehicle-approach walking state; exact return semantics unresolved |
| `0x140b374c0` | Shared transition constructing onboard state with line, stops, vehicle |
| `0x1401710c0`, `0x140b22630` | Add `SimEntityAtVehicle` / `SimPersonAtVehicle`, identified through RTTI |

## Queue selection and assignment

The terminal update calls `0x140b86510` at `0x140b0e357` with line and
boarding-stop index from the current queue. The helper constructs an eight-byte
key and returns that key's vehicle collection or an empty fallback. Queue
lifecycle callbacks independently establish the meaning of the key.

Selection traverses cargo queues and checks stop loading permissions, vehicle
loading state, and capacity against occupancy plus assigned passengers. It does
not establish alternative-line destination compatibility. This explains where
the observed S1 passenger's selection is restricted to S1 vehicles.

Assignment is batched. Disassembly confirms a vehicle-ID store at
`0x140b0f49f` into `SimEntityAtTerminal + 0x18`, between pre-change notification
`0x142bb6970` (call site `0x140b0f442`) and post-change notification
`0x142bb6b50` (call site `0x140b0f4b0`). RTTI `0x143cea8f0` identifies the
component. Observed uses support line at `+0`, boarding index at `+4`, alighting
index at `+8`, arrival time at `+0x10`, and assigned vehicle at `+0x18`.
These offsets are build-specific findings, not a supported write interface.

API-side caller `0x140eecdc0` copies the lookup collection into Lua through
thunk `0x140efff60`. Association with the `getLineStopVehicles` registration
is strongly suggested, but its complete binding glue remains unverified.

## Two indexes and the boarding transition

Component-change callbacks remove/add a passenger in the **vehicle-keyed
reverse index** when the assigned vehicle is nonnegative. They do not migrate
the **line/stop queue**. Entity-added/removal callbacks maintain that queue,
cargo counts, and station membership. Removal locates the passenger by arrival
time with an entity-ID tie-break. An ordinary component-change notification
around a line-field write would therefore leave other indexes inconsistent.

`NoteSimEntityAtTerminalChanged` handles a selected vehicle by cleaning up the
old state and attempting vehicle-approach walking; a path also reaches
`0x140b374c0` directly. When approach walking finishes, `NoteWalkPersonsArrived`
also calls this shared transition. Both pass the existing waiting line and
boarding/alighting indices alongside the selected vehicle. Simply broadening
the vehicle lookup could create an old-line/new-vehicle combination.

The shared transition updates person timing, calls seat-selection helper
`0x140b1c9f0`, and adds both `SimPersonAtVehicle` and `SimEntityAtVehicle` through
engine component registration. RTTI addresses are `0x143cea9c8` and
`0x143cea960`, respectively. The seat helper gathers available slot pairs and
selects one, or returns a pair of `-1` values; exact seat ownership and later
manifest updates still need tracing. Cleanup removes terminal and visual/walking
components through the entity manager. Calling one helper in isolation is not
established as a valid boarding operation.

Special vehicle sentinel values `-1`, `-2`, and `-3` take distinct branches.
Do not treat all negative values as ordinary unassigned waiting passengers.

## Remaining uncertainties

The vehicle-arrival handler updates timing/loading fields before invoking a
signal near its end. Its full relationship to the Lua arrival event and ordering
relative to terminal queue processing remain unverified.

Thread ownership, a safe queue-mutation boundary, object lifetimes, callback
ordering, onward-journey consistency, seat reservation, and failure recovery
are unresolved. Ghidra has incomplete analysis and imperfect signatures; for
example the approach helper is inferred as void although its caller tests AL.
No function signature or address here is sufficient for installing a hook.

The done-waiting lead reaches private-driving end-of-path processing. Do not
treat that generic name as a transit boarding interception point.

## Minimal intervention proposal

The first case remains Test 2's S1/S3 Wimborne-to-Melksham adjacent segment,
with matching boarding and alighting terminals. The
[offline candidate module](../src/interlining/README.md) produces a proposal
from copied snapshots and otherwise retains the current journey. It does not
call game APIs or change the installed passive probe.

A future adapter must revalidate the passenger, route revisions, stop occurrences,
load/unload permissions, capacity/reservations, and onward journey at an
engine-owned boundary. Then it must migrate the whole waiting state while
preserving queue age and allow normal selection/boarding to proceed. Removal
and re-addition are a candidate mechanism, not yet a verified transaction.
Failure must retain the original state without partial index changes. Keep
engine access in a reusable extension and policy in the separate Lua module.

## Next steps and reproducibility

Next trace queue migration and onward-journey ownership, identify a safe callback
boundary, then validate it with read-only runtime instrumentation before writes.
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

Raw follow-up exports include `queue-maintenance.txt`, `queue-lifecycle.txt`,
`boarding-transitions.txt`, `boarding-component-and-binding.txt`, and
`terminal-update.asm` in the ignored evidence directory.
