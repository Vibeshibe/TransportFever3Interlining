# Native vehicle queries — 2026-10-07

## Result and limits

Bridge 0.3 adds read-only vehicle queries through ABI 3, preserving ABI 1/2.
The disposable-save capture completed 400 callbacks, with 10 vehicle snapshots
covering five vehicles on S1/S2/S3, 15 passenger records, 30 configured-stop
records, no reported query errors and zero rejected boundaries. The observer
then disabled itself; the wrapper continues forwarding until game exit.

Vehicle capacity and onboard counts were observed live. **Every sampled
reservation count was zero.** Nonzero reservations, mixed freight/person entries,
stale assignments and overcapacity refusal passed synthetic fixtures; those cases
are not yet independently validated in game. Selected arrival terminals were
read, but passenger waiting-edge access is not yet exposed. No passenger routes,
queues, installed mods or save files were changed. No interlining fix is claimed.

## Static provenance

All addresses below are preferred-image virtual addresses for executable SHA-256
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
Ignored Ghidra exports: `analysis/native-boarding/vehicle-readers.txt`,
`platform-readers.txt`, `platform-bindings.txt`, and `queue-maintenance.txt`.

| Data | Evidence and layout |
| --- | --- |
| TransportVehicle | RTTI `0x143ceaaa0`; typed getter `0x1401856d0` establishes stride `0x1e8`; registration `0x141a36bb0` exposes offsets |
| Capacity/timer | Vectors at vehicle `+0x70` / `+0x148`; element zero is passengers, as used by terminal Update `0x140b0db00` |
| State/line/stop | Vehicle `+0xa8`, line `+0xb8`, stop index `+0xbc`, loading state `+0x138`, user-stopped byte `+0xac` |
| Selected arrival | Station index `+0xc0`, terminal `+0xc4`, decision-locked byte `+0xc8` |
| Loading vehicles | Terminal system `+0x28` points to TransportVehicleSystem; its dynamic object at `+0x1b0` has map `+0x90`, keyed by `{line,stop}`; getter `0x140b86510`, lookup `0x14093e4e0`, entry stride `0x20`, vector at entry `+8` |
| Onboard counts | Terminal system `+0x30` points to SimEntityAtVehicleSystem; dynamic object at `+0x78`, map `+0x60`, keyed by vehicle; getter `0x140b14ab0`, lookup `0x1409c7460`, entry stride `0x20`, cargo-count vector at entry `+8` |
| Boarding reservations | Terminal dynamic object at `+0x78`, reverse map `+0x90`, keyed by vehicle; callbacks `0x140b0c730` / `0x140b0c7f0`; lookup `0x1407ed290`, entry stride `0x38`, entity-ID set at entry `+8`; set insertion `0x140149850` proves 4-byte IDs |

Terminal Update counts passengers already assigned to a vehicle, adds onboard
occupancy, and compares the sum with capacity before selecting more passengers.
The bridge enumerates that reverse index, verifies every referenced waiting
component still names the vehicle, and counts only SimPerson cargo type zero.
It returns the pre-selection remainder; subsequent engine selection can consume
those places. A result is neither a reservation nor permission to board.

`api/tealdef/api/type.d.tl` explicitly defines `StationTerminal.station` as the
zero-based index inside a station group. It is **not** a station entity ID.
Configured stop terminals and selected arrival terminals are separate facts.

Readers copy memory directly: engine getters can assert or detach shared storage.
Bounded Swiss-table scans verify occupancy and reject duplicate matching keys;
reservation sets reject duplicate IDs. Truncation, malformed state, stale indexes,
read-budget exhaustion and unavailable types fail without changing caller output.
SDK details and limits are in [native/README.md](../native/README.md).

## Validation and artifacts

Commands (all used the pinned LLVM-MinGW toolchain):

```sh
rtk proxy python3 tools/build_native_bridge.py --build-dir analysis/native-bridge/build-v3
rtk proxy timeout 45 env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v3/query_tests.exe
rtk proxy timeout 45 env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v3/bridge_tests.exe
```

Both suites passed; compilation used `-Wall -Wextra -Werror`. Tests exercise
contiguous/shared vehicle storage, capacity minus onboard/reserved counts,
freight exclusion, empty/duplicate/malformed maps, stale assignments, nonfinite
timers, overflow refusal, missing vehicle types without breaking ABI 2, output
preservation, budgets and ABI 3 thread/token guards. Existing assembly/registry
and passenger/route tests also passed.

User restarted a disposable baseline save and resumed at normal speed. Explicit
loading with GE-Proton11-5 in the game's existing `.../Transport Fever 3/pfx/`
prefix returned `tfx_start=0`. Windows game PID **312**, simulation thread **592**;
worlds **`0x37dc2200`** and **`0x37dc2ec0`**. Loader calls used outer 45-second
timeouts. No debugger or stop-loader was needed.

Capture: `analysis/native-bridge/build-v3/tpf-bridge-312-26298912.log`.
SHA-256: `b4f24dba0aa03806de94cce0086920773aaca0f05bdbf690cfeb1f4dd12890d1`.
Loaded DLL SHA-256:
`4582aadc3aa5fbaf27fc09778feafd557cf6e535436a0b487409a8b683fe10a0`.
The capture's header still says 0.2/ABI 1,2 due to an outdated log string; the
actual API is ABI 3. That string was subsequently corrected and rebuilt in
`build-v3-final/`; no query or hook code changed after the capture.
Final DLL SHA-256:
`e19b537bc3d99abe63e0f7251f8b9c93fd862d72e8cd4e601e17abe513483f3c`.
The final query suite also passed after adding an explicit mixed-person/freight
reservation fixture (`build-v3-final/query_tests.exe`).

| Vehicle | Line | Observed stop(s) | Capacity | Onboard | Selected terminal |
| --- | --- | --- | --- | --- | --- |
| 97914 | S3 / 97743 | 0 | 40 | 2 | 2 |
| 98388 | S2 / 97742 | 4 | 40 | 1 | 2 |
| 97912 | S3 / 97743 | 2 | 40 | 0 then 2 | 0 |
| 97745 | S1 / 98278 | 1 | 40 | 0 | 1 |
| 98390 | S2 / 97742 | 1 | 40 | 1 then 0 | 2 |

All snapshots had station index zero and a locked arrival choice. All vehicle
line/stop fields matched the queried loading index. The built-in observer cycles
through baseline-specific line IDs; the SDK itself has no hard-coded save IDs.

## Platform lookup traced, not implemented

The scripting `getStationTerminalsForPersonEdge` binding reaches native getter
`0x140b41050` via `0x140eae4c0`, `0x140ee5fb0`, `0x140f02e00` and
`0x140ef92b0`. The latter obtains StationSystem from engine context `+0x158`.
The getter reads a dynamic object at system `+0x38`, then an 8-byte edge-key map
at dynamic `+0x70`, with `0x20`-byte entries and a vector at entry `+8`.
Ignored `platform-native.txt` and the preceding binding/invoke exports preserve
this chain. Vector field meanings, waiting-place edge layout and current-world
system ownership remain to be verified; no client query uses this layout yet.

## Next gate

Finish the passenger waiting-edge to actual station/terminal membership query;
validate its current-world system ownership before exposing it. Capture at least
one nonzero reservation and independently compare the index with waiting records.
Then complete live candidate revalidation, including unloading/journey continuity,
before the engine-owned one-passenger mutation experiment. The pure offline
adjacent-stop policy remains unconnected to the bridge.
