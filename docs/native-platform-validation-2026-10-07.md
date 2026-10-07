# Waiting-edge membership and reservations — 2026-10-07

## Result

Bridge 0.4 adds ABI 4 `read_person_platforms`, preserving the existing API layouts.
It copies the waiting edge/place and direct station/terminal memberships from
StationSystem in the **current callback's world**. No engine getters or mutation
operations are called. Passenger behavior remains unchanged.

In the disposable baseline, the query distinguished S1/S3's shared Wimborne
terminal from S2's separate terminal. The same capture observed a passenger
assigned to a vehicle whose reverse index reported a nonzero reservation,
closing the previous live-evidence gap. This validates those sampled reads,
not the complete cross-line boarding operation.

## Static provenance

Executable SHA-256:
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
Addresses below use preferred base `0x140000000`; the DLL relocates RVAs.

- RTTI `.?AVStationSystem@ecs@@` is at RVA `0x3d972a0`; complete-object locator
  `0x14390eee8` identifies vtable **`0x143706600`**.
- Dispatcher `0x142bb8a50` walks the world's system vector at **world `+0x30`**
  with 8-byte system pointers and invokes their Update methods. The reader scans
  that vector for one StationSystem vtable match. It rejects ambiguous/missing
  ownership and caches the pointer only within one dispatch. This avoids relying
  on the scripting engine context's system pointer across alternating worlds.
- `0x14195ec60` registers SimPersonAtTerminal: **edgeId at `+0`, place at `+8`**.
  The component is 12 bytes; EdgeId is `{entity, index}`, two 32-bit integers.
  The public Teal declarations describe the edge as an index within a transport
  network entity, not a station terminal index.
- Native getter **`0x140b41050`** reads StationSystem's dynamic object at `+0x38`
  and its person-edge map at **dynamic `+0x70`**. Keys are 8-byte EdgeIds;
  `0x14093e4e0` establishes 32-byte map slots with a vector at slot `+8`.
- Station insertion **`0x140b3fe60`** walks 0x88-byte terminals. Terminal person
  edges at `+0x38` feed this map; each value is the station **entity** and terminal
  index, 8 bytes total. Station-wide person edges are associated with every
  terminal. Multiple memberships are legitimate; duplicate identical pairs are
  rejected by both engine assertions and the bridge reader.

Ignored evidence: `analysis/native-boarding/platform-system.txt` and
`platform-place-layout.txt`, plus the previously documented binding chain ending
in `platform-native.txt`. Original game files were only read.

## API and limits

Request `TFX_PLATFORM_ABI_VERSION` using `tfx_api_v4`; see
[native/README.md](../native/README.md). Results contain up to 16 station-entity/
terminal pairs. No direct mapping yields a successful empty result. Missing
person/waiting/place components return `TFX_NOT_FOUND`; unsupported system lookup
returns `TFX_QUERY_UNAVAILABLE`; malformed or duplicate data fails. Oversized
results return `TFX_LIMIT_REACHED` without truncation. Failed queries preserve
caller output. Existing callback/thread and shared read-budget rules apply.

The system vector is bounded to 256 entries and edge maps to 4,095 slots.
These are conservative pilot limits, not promises about every possible save.
Station pointers and memory copies are never retained as eligibility evidence
across callback invocations or worlds.

Engine helper **`0x140952760`** tries edge membership, then endpoint-node
membership through `0x140b41150` if the edge map is empty. ABI 4 currently exposes
only the direct edge map. Empty membership must not authorize an alternate
boarding. Future support may add that fallback; the initial policy can instead
reject such candidates. Destination unloading, terminal flags, actual locked
vehicle arrival, maximum-load rules and journey continuity remain separate checks.

## Tests

The Windows build was warning-clean (`-Wall -Wextra -Werror`). Both native suites
passed under the isolated Wine prefix. New fixtures cover:

- Multiple valid terminal memberships and no direct mapping for a different edge.
- Missing waiting place, invalid place/pair, oversized result and unchanged output.
- Missing/duplicate StationSystem ownership and malformed system vectors.
- Shared component storage and unreadable/budget-exhausted data.
- Two worlds with the same person/edge IDs but different station mappings.
- ABI 4 callback-token and thread guards, while ABI 1–3 remain available.

```sh
rtk proxy python3 tools/build_native_bridge.py --build-dir analysis/native-bridge/build-v4
rtk proxy timeout 45 env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v4/query_tests.exe
rtk proxy timeout 45 env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all wine analysis/native-bridge/build-v4/bridge_tests.exe
```

## Runtime capture

The user restarted a disposable baseline save and resumed at normal speed after
loading returned `tfx_start=0`. Game Windows PID **308**, simulation thread **592**;
alternating worlds **`0x2cd5ea30`** and **`0x2cd5f6f0`**. The same GE-Proton11-5
runtime and existing game prefix were used. No debugger was attached.

Loaded DLL: `analysis/native-bridge/build-v4/tpf_shared_bridge.dll`.
SHA-256: `430a3fca76041a2a0c89a6a172fa0fb91968f7f9fecef9b6665fc3c337fceeb0`.
Capture: `analysis/native-bridge/build-v4/tpf-bridge-308-27811482.log`.
SHA-256: `cb87229472cebc117fdbb9da2180cac168c95991b6eb8baca1ff4f582958699e`.
Completed **1,200 callbacks**, with nine platform snapshots, 15 Wimborne line
comparisons, 64 vehicle snapshots across ten vehicle IDs, one assignment
cross-check and four nonzero-reservation vehicle samples. No query errors were
logged; final boundary rejection count was zero. The completion record confirms
observation auto-disabled. No stop-loader was needed; the DLL remains forwarding
until game exit. User was told they can pause or close the game.

At sequence **595**, waiting passenger **71583** reported S3 line **97743**, stops
**4→5**, assigned vehicle **97913**. A separate vehicle API call in that callback
reported capacity **40**, onboard **0**, reserved **1**, remaining **39**. Vehicle
samples also showed nonzero reservations at sequences **9**, **598**, and **604**.
This corroborates a live assignment against the vehicle reverse index; it is not
an exhaustive census of every reserved passenger or every possible cargo case.

Physical membership examples at Wimborne station **97948**:

| Passenger | Waiting line | Waiting edge | Terminal | Configured lines sharing this terminal |
| --- | --- | --- | --- | --- |
| 75514 | S2 / 97742 | 97948,338 | 2 | S2 |
| 98760 | S1 / 98278 | 97948,322 | 1 | S1 and S3 |
| 113752 | S1 / 98278 | 97948,309 | 1 | S1 and S3 |
| 114323 | S3 / 97743 | 97948,317 | 1 | S1 and S3 |

These comparisons concern physical membership only. They do not establish that
any listed passenger has a compatible destination on an alternative vehicle.
The research consumer caches only bounded discovery IDs and logging snapshots;
all substantive reads occur again within the live callback. Faster rechecks,
a 1,200-callback bound and suppressed timer-only vehicle changes helped capture
reservations missed by the previous sparse observer.

## Next implementation gate

Build a read-only candidate validator combining current waiting leg, direct edge
membership, actual locked vehicle terminal, loading capacity and adjacent journey
continuation. Add verified terminal load/unload flags and destination checks.
Only after those predicates are validated should the engine-owned one-passenger
replacement experiment be enabled. Preserve queue age and notification/replication
rules from [the adapter design](native-adapter-design.md). No mutation API or Lua
binding has been added by this milestone.
