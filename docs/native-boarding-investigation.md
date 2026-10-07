# Native boarding investigation

Task 004 is active. These are static findings for executable SHA-256
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
Addresses are preferred virtual addresses with image base `0x140000000`.
The observer has runtime-validated selected call arguments and mutation flags;
none is yet a validated behavior-changing patch site.

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
`0x140b1c9f0`, and adds `SimPersonAtVehicle` when a valid seat pair is returned.
It adds `SimEntityAtVehicle` independently of that seat-success branch through
engine component registration. RTTI addresses are `0x143cea9c8` and
`0x143cea960`, respectively. The seat helper gathers available slot pairs and
selects one, or returns a pair of `-1` values; exact seat ownership and later
manifest updates still need tracing. Cleanup removes terminal and visual/walking
components through the entity manager. Calling one helper in isolation is not
established as a valid boarding operation.

Special vehicle sentinel values `-1`, `-2`, and `-3` take distinct branches.
Do not treat all negative values as ordinary unassigned waiting passengers.

## Existing stop-remapping mechanism

`SimPersonSystem::NoteLineChanged` at `0x140b307b0` provides an engine-owned
example. It maps old stop occurrences to new indices and validates the mapped
stops. In its successful waiting-state branch, disassembly at `0x140b3113d`
copies the line, replacement stop indices, original arrival-time bytes, and
vehicle into a temporary component. It removes the old component through
`0x142bb7320` at `0x140b31168`, then adds the replacement through `0x140b21cd0`
at `0x140b3117e`, with notification/storage flags enabled.

This establishes a removal/re-addition pattern for **stop remapping on the same
line**. Cross-line reassignment and rollback are not established by that example.
The add helper copies 32 bytes, updates the component mask/version, and notifies
systems. `SimPersonSystem::ComponentAddedToEntity` at `0x140b2bcb0` also indexes
terminal passengers by line through `0x140951180`; queue indexes are not the only
dependents. Its index storage uses a copy-on-write helper at `0x140b39cf0`.

Onward travel remains destination-driven in the inspected alighting path:
`0x140b25e30` reads `SimPerson.targetOrAtEntity` (native `+0x50`), resolves the
onboard line/stop location, builds destination inputs through `0x140952920`,
and calls `0x140b378e0` to construct the next path state. Invalid-destination
branches use cleanup/fallback. This is evidence against assuming the waiting
component contains a complete fixed itinerary; transfer correctness still needs
an actual reassignment test.

## Modification windows and replication

Diagnostic strings identify `0x142bb4870` as
`ecs::Engine::CheckAddComponentPreconditions`. It requires `m_betweenChanges`
at engine `+0x1f0` to exist. During a notification it additionally requires
`m_allowPostAndNestedChangesHack` at `+0x1f8`; the notification flag is at
`m_betweenChanges + 0x50`. Removal performs the same checks. These assertions
do not license changing either flag or mutating from an arbitrary thread.

| Entry | Static role |
| --- | --- |
| `0x142bb4640` | `BeginModification`; requires no existing modification window |
| `0x142bb4d90` | `EndModification`; drains posted callables before completing changes |
| `0x142bb6ea0` | `Post`; requires an existing window and the nested/post flag |
| `0x142bb4430` | Replays recorded entity/component operations into another engine |
| `0x142bb78f0` | `Replicate`; calls the replay helper |

`Post` is not a general cross-thread scheduler. It stores native C++ callable
objects, whose ABI/lifetime must not be guessed. Replica replay includes begin,
end, add, remove, and change operations. The live trace contains removal return
address `0x142bb455e` and onboard-add return address `0x142bb44fe` inside this
replay helper. Never persist a world pointer as a universal authoritative world
or classify repeated passenger IDs in different worlds as duplication.

## First native runtime observation

The [GDB observer](../tools/native_trace/README.md) passed live executable/hash
checks under Proton and captured person `71583` on S3 (`97743`), segment 3→4,
vehicle `97914`. Waiting state, approach-state replacement, the common boarding
entry, and onboard-component addition matched the static ABI. The observation
used hardware execution breakpoints and no engine calls or passenger-state writes.

All nine logged transition records ran on Linux simulation thread `75558` in
process `74799`. Two engine-world pointers appeared, with replay call sites
explaining the second-world operations. Every record had a non-null modification
window and notification flag zero; direct transitions had the nested/post flag
one, while observed replay removals/onboard addition had zero.

This validates the observer and an ordinary S3 boarding path, not interlining,
stop-edit migration, or a write-capable hook. The generic remove breakpoint
reached 5,001 total hits after roughly 11 seconds; only nine concerned the traced
transitions. Debugger overhead invalidates performance/timing comparisons.
See [runtime evidence](native-runtime-observation-2026-10-07.md).

## Remaining uncertainties

The vehicle-arrival handler updates timing/loading fields before invoking a
signal near its end. Its full relationship to the Lua arrival event and ordering
relative to terminal queue processing remain unverified.

The observed transitions run on the simulation thread, but a safe interception
boundary, world ownership across ticks, object lifetimes, callback ordering,
cross-line onward travel, seat reservation, and failure recovery remain unresolved.
Ghidra has incomplete analysis and imperfect signatures; for
example the approach helper is inferred as void although its caller tests AL.
No function signature or address here is sufficient for installing a hook.

The done-waiting lead reaches private-driving end-of-path processing. Do not
treat that generic name as a transit boarding interception point.

## Minimal intervention proposal

The candidate insertion point is now **entry to terminal Update**, with a
separate modification scope completed before calling the original function.
Two boundary captures validated 280 complete calls with no active modification
window at entry/return. The dispatcher passes each invocation's world; its
identity alternates. See [the concrete adapter design](native-adapter-design.md).

The existing commit scope starts only after candidate selection. Its constructor
`0x142bbbba0` calls BeginModification; destructor `0x142bbc060` reaches
EndModification. Post-construction and post-destruction sites are `0x140b0f40e`
and `0x140b0f5ef`; neither fired in the boundary samples. Migrating queues there
would be too late for the assignment list already collected.

Person-system removal callback `0x140b2c450` reads the old terminal line and
calls `0x140953820`, asserting exactly one line-index removal. Its addition
counterpart calls `0x140951180`, asserting insertion. This closes the previously
untraced removal side of that index; a cross-line write is still untested.

The first case remains Test 2's S1/S3 Wimborne-to-Melksham adjacent segment,
with matching boarding and alighting terminals. The
[offline candidate module](../src/interlining/README.md) produces a proposal
from copied snapshots and otherwise retains the current journey. It does not
call game APIs or change the installed passive probe.

A future adapter must revalidate the passenger, route revisions, stop occurrences,
load/unload permissions, capacity/reservations, and onward journey at an
engine-owned boundary. Then it must migrate the whole waiting state while
preserving queue age and allow normal selection/boarding to proceed. Removal
and re-addition match the engine's same-line stop-remapping mechanism; applying
them across lines is not yet a verified transaction.
Failure must retain the original state without partial index changes. Keep
engine access in a reusable extension and policy in the separate Lua module.

## Next steps and reproducibility

Next implement the pinned native bindings and a shared extension that owns the
selected entry interception. Validate it first without mutation, then perform
the one-passenger cross-line replacement experiment. Select the writable world
from each callback, never from a cached pointer.
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
Further exports are `line-change-transitions.txt`, `queue-migration.txt`,
`entity-notifications.txt`, `person-component-lifecycle.txt`,
`engine-change-replay.txt`, and `engine-modification.txt`. The isolated export
at `0x142bb4453` is a fragment; use complete replay entry `0x142bb4430`.
Boundary follow-up exports: `terminal-scope.txt`, `person-index-removal.txt`, and
`system-update-dispatch.txt`. The latter includes the dispatcher and paired
line-index insertion/removal helpers.
