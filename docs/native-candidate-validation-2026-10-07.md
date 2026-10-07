# Read-only candidate validation — 2026-10-07

## Scope and implementation

Bridge 0.5 adds ABI 5 terminal permission and entity-presence queries, preserving
ABI 1–4. `src/interlining/native_candidate.c` consumes only this public API.
The bridge still exposes no passenger mutation operation. The installed Lua probe,
shipped game files and saves were not modified by this work.

The validator requires an unassigned waiting passenger, adjacent forward leg,
matching configured boarding and alighting station/terminal pairs, direct physical
platform membership, a locked actual arrival and a vehicle in ready Loading state.
It checks boarding passenger load and alighting unload permissions, line load mode
0 with maximum passenger load exactly 1.0, destination presence, loading-index
membership and capacity after onboard passengers plus assigned reservations.

Positive results copy expected passenger state, including destination, original
arrival-time bits and waiting place, alongside proposed line/stop indices and the
current dispatch sequence. Every result sets `requires_engine_revalidation=1`.
Query errors preserve caller output; policy exclusions have explicit reasons.

## Static provenance

All addresses below refer to the pinned executable's preferred base `0x140000000`;
the reader applies its runtime relocation. Executable SHA-256:
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.

- Station RTTI `0x143cea860`, typed getter `0x1405d9640`: component stride
  `0x58`, contiguous/shared storage; its first vector contains terminals.
- StationSystem insertion `0x140b3fe60` and terminal registration `0x1422c1ab0`:
  terminal stride `0x88`, `passengersLoad` at `+0x61`, `passengersUnload` at
  `+0x63`. These are distinct from cargo flags at `+0x60` / `+0x62`.
- Vehicle load-state registration identifies Loading as 3, between Unloading 2
  and WaitForScheduledDeparture 4. The validator additionally requires state 2
  (AT_TERMINAL), no user stop, and a finite passenger loading timer at or below zero.
  Terminal Update's native ready gate compares against zero.
- Destination is Person `+0x50`. Native alighting `0x140b25e30` uses this and
  onboard line/stop through `0x140952920` / `0x140b378e0`. The presence reader
  checks the entity's nonempty component row against its 128-bit mask, without
  invoking a pathfinding routine.
- Fractional maximum-load allocation involves compartment/configuration helper
  `0x142685ad0`; rounding and allocation are unresolved. The pilot rejects
  fractional limits rather than assuming the Lua prototype's floor calculation.

New ignored export: `analysis/native-boarding/terminal-permissions.txt`.
Supporting earlier exports: `platform-readers.txt`, `platform-system.txt`,
`entry-functions.txt`. No decompiled game code is committed.

## Validation procedure

Built with the pinned LLVM-MinGW toolchain and warnings as errors:

```sh
rtk proxy python3 tools/build_native_bridge.py --build-dir analysis/native-bridge/build-v5
rtk proxy env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v5/query_tests.exe
rtk proxy env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v5/bridge_tests.exe
rtk proxy env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v5/candidate_tests.exe
```

All three suites passed. New query fixtures cover separate load/unload flags,
shared pages, invalid flags, terminal bounds, row/mask inconsistency, duplicates,
empty/missing entities, budgets and ABI 5 context guards. Candidate fixtures cover
an accepted route with different line stop indices, preserved expected state,
platform/destination/permission/capacity exclusions, loading timers and query errors.
Existing ABI, registry, slot ownership and x64 forwarding tests also passed.

The user restarted, loaded a disposable baseline paused, then unpaused at normal
speed. GE-Proton11-5's loader returned zero into Windows PID 308. Artifact:
`analysis/native-bridge/build-v5/tpf_shared_bridge.dll`, SHA-256
`808c2c18ff170725c3fab3e487da2b1bd20735b22afaebe5e65807fc14782c13`.
Capture: `analysis/native-bridge/build-v5/tpf-bridge-308-29658466.log`.

## Live observations

Completed **1,200 accepted callbacks** on simulation thread **596**, alternating
worlds `0x2c5d3aa0` and `0x2c5d4760`. Final summary:

| Measure | Count |
| --- | ---: |
| Remembered people / vehicles | 15 / 11 |
| Ready vehicle samples | 45 |
| Candidate evaluations | 27 |
| Positive evaluations | 7 |
| Unsupported-leg rejections | 12 |
| Boarding-mismatch rejections | 8 |
| Query errors / rejected boundaries | 0 / 0 |

Ten changed results were logged; all seven positives concern the same observed
person/vehicle pair described below. Counts include repeated evaluations, not
unique people, logical ticks or boardings. The final log confirms observation
is disabled and the hook only forwards. No debugger or stop-loader was used.
The DLL remains pinned until process exit; restart before loading a new version.
Log SHA-256: `14347b5961e4769026a0ac3f6a02833f1e7129dc49a9eada578e285f1db430f3`.

At sequence 47, person **22747** waiting for S1 (`98278`), stops **1→2**, passed
all validator checks for S3 (`97743`) vehicle **97959**, also stops **1→2**.
Boarding resolved to station **97948**, terminal **1**; alighting resolved to
station **97294**, terminal **0**. Destination entity **86002** was present;
remaining capacity was **40**. This is a read-only proposal, not a boarding.

At sequence 302, person **86997**'s S3 leg **0→2** was rejected as unsupported;
person **87485**'s S3 leg **0→1** was rejected for a boarding mismatch against
S1 vehicle **98398**. No invariants were relaxed to produce a positive result.

## Limits and next experiment

A matching alighting terminal preserves the intended connection point in the
proposal; destination presence does not prove onward path validity. Actual
cross-line boarding, destination completion, replication and save/reload remain
untested. Multi-stop legs, wraparound, alternative terminals, node-based platform
fallback and fractional/full-load modes are excluded. Research discovery is
bounded to remembered IDs and the baseline's three lines; it is not exhaustive.
Repeated positives can include the same person and both engine worlds.

Next implement one explicitly bounded engine-owned waiting-component replacement
using the verified modification scope and lifecycle operations. Revalidate current
expected state immediately before it; preserve arrival-time bytes and vehicle -1.
Finish the dedicated scope before original terminal selection. Then observe
boarding, alighting, onward travel and replica consistency on a disposable save.
No direct field writes, seat reservation or mutation has occurred in this capture.
