# Passenger Interlining Investigation

## Status

Initial static feasibility assessment, 2026-10-06. No working fix, runtime
experiment, or decompilation has been completed. Game binaries and content
archives were not modified. Ghidra was not installed during this assessment.

True opportunistic boarding appears to require native engine work unless further
investigation reveals a suitable scripting hook. This is an inference from the
available declarations and scripts, not proof that no such hook exists.

## Evidence from this installation

- `api/tealdef/api/engine.d.tl:4` explicitly describes engine access as read-only.
- `api/tealdef/api/engine.d.tl:1002`: `SimEntityAtTerminal` records a single
  `line`, boarding stop `lineStop0`, alighting stop `lineStop1`, arrival time,
  and vehicle. This is consistent with line-specific waiting, but does not
  establish the complete boarding algorithm.
- `api/tealdef/api/type.d.tl:2346`: `PathFactoryInput` has movement-mode cost
  factors. `PathFactoryOutput` at line 2365 has concrete line sections and cost.
  Declarations alone do not provide a supported way to alter live paths.
- `api/tealdef/api/engine/system.d.tl:133`: passenger queries support inspecting
  waiting passengers and line usage; no route reassignment method appears here.
- `api/tealdef/api/cmd.d.tl:994`: debug commands can set person state or happiness.
  The component exchange command documents only emission/model components.
  These are not established passenger rerouting interfaces.
- Inside `base/content/game_mechanics.zip`, file
  `game_mechanics/towns/towns.script.tl:371–393` handles
  `SimPersonSystem/OnModifyPathfinding`. Its declared payload contains
  `town2lineCostFactor`, used for town marketing. No per-passenger or individual
  transit-line selector appears in that handler. The native event producer
  still needs inspection to establish the full payload and timing.
- `BaseConfig` exposes passenger happiness decay and destination recomputation
  probability. Their names do not establish control over alternative boarding.

The official [engine documentation](https://wiki.transportfever3.com/script-doc/api/engine.html)
also describes read-only access; the [command documentation](https://wiki.transportfever3.com/script-doc/api/cmd.html)
is a reference for supported mutations.

## Native investigation starting points

Inspected executable: `TransportFever3.exe`.

SHA-256: `5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`

PE preferred image base: `0x140000000`. The following are **file offsets of
diagnostic strings**, not function addresses or patch locations:

| File offset | String |
| --- | --- |
| `0x36e61d8` | `PathFactory::Compute` |
| `0x3701e98` | `ecs::SimEntityAtTerminalSystem::Update` |
| `0x3704950` | `ecs::SimPersonSystem::NoteSimEntityNeedsPathChanged` |
| `0x37049c0` | `ecs::SimPersonSystem::NoteSimPersonDoneWaiting` |
| `0x370a6e0` | `ecs::TransportVehicleSystem::HandleVehicleArrived` |
| `0x372b610` | `town2lineCostFactor` |
| `0x372b828` | `OnModifyPathfinding` |

These strings and retained C++ RTTI make targeted analysis plausible. Locate
their cross-references in Ghidra, inspect the event producer and boarding
eligibility checks, then verify behavior at runtime. Strings alone do not
confirm a safe interception point. Ghidra MCP is optional tooling.

## Proposed behavior

Start with passengers whose planned alighting stop is served by multiple lines
from the same accessible boarding location, in the correct direction. Allow
an arriving service only if capacity, loading rules, and downstream connectivity
permit it. Map stop indices to the replacement line and preserve the onward
journey. A shared station name is insufficient when platforms differ.

Compare boarding now with waiting using expected remaining journey time,
transfers, walking, crowding, and comfort. Give passengers stable, reproducible
preference weights. Express comfort penalties relative to ride duration so a
short trip can favor immediate departure while a long trip can justify waiting.
Randomizing a single preselected line would not by itself provide opportunistic
boarding.

Cache eligible alternatives by boarding/alighting pair and direction; invalidate
them when services change. Reevaluate at vehicle arrivals rather than running
full-network routing for every passenger every frame. Correct implementation
must update queue membership, passenger state, and vehicle manifests through
the engine's transition mechanisms, not just overwrite the visible line field.

## Validation gates

Use a disposable save with the illustrated three-line shared trunk. Confirm
baseline behavior before introducing changes. Test short shared-stop trips,
destinations beyond each branch, opposite directions, full vehicles, different
platforms, express services, line edits, transfers, and save/reload. Measure
waiting time, end-to-end travel time, boardings per line, stuck passengers, and
simulation cost. Reject changes that merely spread loads while breaking trips.

An engine prototype must target the inspected executable version, verify its
compatibility before activation, and leave the original executable recoverable.
