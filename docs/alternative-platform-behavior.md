# Alternative-platform passenger behavior

Research date: 2026-10-09. Static findings for executable SHA256
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
No debugger attachment, game edits or replacement DLL were used. The independent
0.9.1 same-platform trial continued throughout this investigation.

## Documented behavior

The [official line-management manual](https://wiki.transportfever3.com/doku.php?id=gamemanual:management:linesvehicles)
says a vehicle can use an alternative terminal when its primary terminal is
occupied. Trains select at the last signal before the station; all enabled
alternatives must be reachable from that signal. Road vehicles select at the
station entrance. These are vehicle-routing rules: the manual does not specify
passenger transfer paths, walking-time costs or missed-train handling.

## What the native code establishes

The ordinary positive-vehicle branch of `SimPersonSystem::NoteSimEntityAtTerminalChanged`
has two outcomes after a passenger receives a vehicle assignment:

1. If a suitable vehicle-approach path exists, create walking state, including a
   `ModelPerson` component. `NoteWalkPersonsArrived` subsequently performs the
   onboard transition for a passenger still assigned to a nonnegative vehicle.
2. If that approach cannot be constructed, take the common onboard transition
   immediately. Missing waiting-place state also reaches direct boarding.

The approach builder examines the assigned vehicle's transformed model entry
paths under `transportVehicle` metadata. It selects the nearest entry whose
first point is **strictly less than 5 game-world units** from the passenger's
waiting position. It constructs links from that position through the entry
points. This particular builder performs no station pedestrian-network search.
An empty result returns false, and the assignment caller falls through to the
direct onboard handler.

This is evidence of a short boarding walk and a real no-walk fallback, rather
than evidence that every alternative-platform passenger teleports. An upstream
operation could already have relocated or rerouted the passenger before this
handler. That part of the alternative-platform sequence is unresolved.

During this research the user visually observed a passenger apparently teleport
into a vehicle at an excessively long platform. This is consistent with the
distance-limited approach and direct-board fallback. No synchronized position,
assignment or timing trace was taken, so it does not identify the exact branch
or demonstrate zero simulation-time cost. The user's hypothesis that alternative
platforms behave similarly is plausible through the same assignment handler.
The user clarified that they are certain of this behavior in **Transport Fever 2**.
Treat that as predecessor-game precedent, not confirmation of TF3 alternative
platform behavior; the latter remains unverified.

The shipped `api/tealdef/api/engine.d.tl` describes `ModelPerson.path`, `pathPos`,
`speed`, `topSpeed`, `stopped` and timing fields. The arrival callback shows that
the walking branch has a distinct completion event. It does **not** establish
that route choice charges the passenger for an entire cross-platform walk, or
that a train waits long enough for that walk to finish.

## Evidence map

Addresses use the executable's preferred base `0x140000000`, not live pointers.

| Address | Evidence |
| --- | --- |
| `0x140b285d0` | Assignment handler; obtains waiting position through `0x1409523e0`. |
| `0x140b28a6b` | Calls approach helper `0x140b38950`; `test al,al` at `0x140b28a70` controls fallback. |
| `0x140b28ac0` | Calls common onboard handler `0x140b374c0` on approach failure. |
| `0x142694360` | Initializes empty path and nearest-entry threshold from float `5.0` at `0x143678cdc`. |
| `0x142692cb0` | Enumerates vehicle models/transforms for the entry search. |
| `0x1426933e0` | Transforms entry points, compares Euclidean distance and constructs links; strict comparison at `0x14269358e`. |
| `0x140b38950` | Empty path returns false; successful path creates `ModelPerson` through `0x14016a690`. |
| `0x140b33a30` | Walking-arrival handler; assigned passenger reaches common onboard call at `0x140b34713`. |

Ghidra incorrectly inferred a void return for the approach helper. Assembly
confirms its boolean return and caller branch; do not copy the inferred signature
into an SDK binding. RTTI at `0x143cf0a98` identifies `ModelPerson`.

Also inspected `NoteMovePathChanged` (`0x140b31d50`), `NoteVehicleChanged`
(`0x140b33470`) and vehicle update/notification dispatch. None established the
complete alternative-platform transfer sequence. In particular, cleanup for
vehicle states other than EN_ROUTE/AT_TERMINAL does not prove recovery when an
ordinary train departs before a walking passenger arrives.

Ignored exports, disassembly and decoded constants are under
`analysis/native-boarding/alternative-platform-*` and `alternative-*.asm`.
`alternative-platform-evidence.json` records their hashes; manifest SHA256:
`495775dc4fe2deb2cdbd20cde3058f0f6a87ce4ef89795bbda919c0fae13db27`.
Reproduce targeted exports using the existing read-only project, for example:

```sh
rtk proxy env GHIDRA_HEADLESS_MAXMEM=3G python3 tools/ghidra_headless.py \
  analysis/ghidra TPF3Interlining -process TransportFever3.exe \
  -noanalysis -readOnly -scriptPath tools/ghidra \
  -postScript ExportFunctions.java analysis/native-boarding/alternative-recheck.txt \
  0x140b285d0 0x140b38950 0x142694360 0x1426933e0 0x140b33a30
```

## Consequences for interlining

Keep the current checks in `src/interlining/native_candidate.c`: alternative
endpoint exclusion, matching terminals, locked actual arrival and waiting-edge
membership. Removing them would allow selection without proving a physical walk;
the native fallback could complete boarding directly.

Cross-platform support should resolve the train's actual selected platform,
obtain a reachable pedestrian path, account for its walking duration and a
boarding margin, and preserve a valid journey when the train leaves or changes
platform again. Reuse a verified native reroute operation through the shared
bridge where available. The short vehicle-entry helper alone is insufficient.
Whole-journey cost comparison remains separate work.

## Focused follow-up after the current trial

Use a disposable same-line setup with the primary platform occupied. Compare a
normal primary-platform boarding against nearby and distant reachable alternatives,
and a deliberately disconnected pedestrian platform if the editor permits it.
Record selection at the decision signal, locked terminal, passenger position and
path, assignment/reservation, walking completion, onboard state and simulation
time. Include a departure before walking completion. Observe both worlds and use
bounded logging without debugger pauses for timing.

These measurements distinguish advance walking, position relocation and direct
boarding. They also test departure recovery. Planner cost accounting needs its
own evidence: visible walking or an elapsed boarding delay alone cannot prove it.

## Subsequent implementation decision

The user requested matching native behavior rather than adding walking-time
simulation. Version0.10.0 prepares a bounded experiment for cross-platform
boarding and different fixed alighting platforms inside the same station.
It validates explicit selected boarding options, terminal permissions and fresh
route snapshots, and delegates onward path construction to the engine. This
supersedes the earlier proposal to require a new pedestrian router first; it
adds no claim that the engine simulates platform walking. See
[cross-platform scope and validation](cross-platform-trial.md).
