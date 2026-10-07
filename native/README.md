# Shared native bridge — experimental read-only pilot

The bridge provides one centrally owned terminal-update interception and a small
versioned C API and copied passenger/line-stop/vehicle queries. **It cannot change
passenger routes or components.** The Lua
policy in `src/interlining/` remains separate and is not connected to this DLL.
This is a development loader, not an automatically installed game mod.

## Build and isolated tests

On Linux x86-64, with Python 3.12+, CMake, Ninja, and Wine:

```sh
rtk proxy python3 tools/build_native_bridge.py --fetch-toolchain
rtk proxy env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all \
  wine analysis/native-bridge/build/bridge_tests.exe
rtk proxy env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all \
  wine analysis/native-bridge/build/query_tests.exe
```

Downloads are explicit. The helper pins LLVM-MinGW 20250910 (LLVM 21.1.1), verifies
the release archive SHA-256, and extracts only into ignored `tools/vendor/`.
Subsequent builds omit `--fetch-toolchain`. `--toolchain PATH` selects another
installed LLVM-MinGW directory. Builds and runtime logs stay under ignored
`analysis/`. Do not overwrite a DLL currently loaded in the game; use a new build
directory and restart the game before testing a changed bridge.

Outputs: `tpf_shared_bridge.dll`, `bridge_loader.exe`, `bridge_tests.exe`, `query_tests.exe`, and
`bridge_fixture.exe`. The fixture runs for 60 seconds and prints its Windows PID.
Loading the production DLL there must return status `9` (unsupported build).
There is no production switch that bypasses executable verification.

## Explicit loading under Windows/Proton

Use a disposable save. Run the loader in the **same Proton version and prefix**
as the game; Linux and Windows PIDs differ. The loader supports:

```text
bridge_loader.exe --list
bridge_loader.exe WINDOWS_PID Z:\absolute\path\tpf_shared_bridge.dll
bridge_loader.exe WINDOWS_PID Z:\absolute\path\tpf_shared_bridge.dll --stop
```

Use an existing absolute Windows DLL path under 260 characters. Loading is two
steps: `LoadLibraryW`, then `tfx_start` on a separate remote thread after library
initialization. Remote-thread waits time out after 30 seconds; they never terminate
a timed-out target thread or unload possibly running code. An argument allocation
may remain until process exit on such a timeout. No proxy DLL, launch override,
game-file replacement, or persistence is installed.
Wine startup and other API stalls are outside that timer. Wrap manual Proton
invocations in an outer timeout (for example `rtk proxy timeout 45 ...`). If a
loader times out, do not infer the target operation failed or unload its DLL.

The bridge writes `tpf-bridge-WINDOWS_PID-UPTIME.log` beside the DLL. Start returns
zero only after verifying the executable, live entry bytes, table layout, and
hook ownership. Run the simulation to collect up to 400 accepted invocations;
the first eight and final invocation are logged. This is not a game-tick count:
two engine worlds can alternate. After the limit, the wrapper forwards without
observation. `--stop` also disables future observations; an in-flight callback
may finish. **Restart the game to remove the wrapper or retry initialization.**
The DLL is pinned until process exit. It does not save persistent state.
Version 0.3 registers the bounded `research.passenger-route-observer` client.
It scans at most 1,024 entity masks per invocation and logs at most 24 passenger
samples and their boarding/alighting stops. It stops scanning after eight scan
or waiting-read failures. All its data access goes through the public API.
Its vehicle sampler cycles through the disposable baseline's S1/S2/S3 entity IDs
`98278`, `97742`, `97743` and their stops, logging at most 48 changed snapshots or
eight errors. These save-specific inputs live only in the research consumer;
the SDK accepts caller-supplied IDs. Repeated snapshots are suppressed for logging
only, never reused as eligibility evidence across worlds.

## Client contract

[`include/tpf_bridge.h`](include/tpf_bridge.h) is the ABI. Resolve `tfx_get_api`
from the loaded DLL; request ABI 1 with `tfx_api.struct_size = sizeof(tfx_api)`.
ABI 1 exposes `TFX_CAP_TERMINAL_OBSERVATION` and retains its original layout.
Registration copies a
`tfx_client` containing an ASCII ID, callback and caller-owned user-data pointer.
Keep the callback module and user data alive until unregistration succeeds.
Example callback:

```c
static tfx_api bridge = {.struct_size = sizeof(tfx_api)};
static void before_selection(tfx_context context, void *user_data) {
    (void)user_data;
    tfx_observation sample = {.struct_size = sizeof(sample)};
    if (bridge.read_observation(context, &sample) != TFX_OK) return;
    /* Consume a copied invocation sequence and delta-time bits. */
}
```

Clients run lexically by ID, independent of registration order. Duplicate IDs,
more than 32 clients, and registration changes during dispatch are rejected.
Contexts expire at the end of **each client's** callback. A previous client's
token, another thread, or nested dispatch cannot access the current observation.
No world/component/function pointer crosses the API. Callbacks must return
promptly and normally: this native interface does not contain malicious code,
access violations, exceptions, or hung clients. There is no Lua binding yet.

## Passenger and line-stop queries (ABI 2)

Initialize `tfx_api_v2 api = {.base.struct_size = sizeof(api)}` and call
`tfx_get_api(TFX_QUERY_ABI_VERSION, &api.base)`. ABI 2 adds
`TFX_CAP_PASSENGER_ROUTE_QUERIES` and these operations:

| Operation | Copied result |
| --- | --- |
| `read_waiting_leg(context, person, out)` | Line, boarding/alighting indices, vehicle, destination, original arrival-time bits, and optional opaque terminal-place bytes |
| `read_line_stop(context, line, stop, out)` | Stop count, configured group/station/terminal, loading mode, passenger loading flag and maximum-load fraction bits |
| `scan_waiting_people(context, first_entity, out)` | Up to 16 candidate person IDs after examining at most 1,024 masks; continuation cursor or `-1` at end |

Set every result's `struct_size`. Failed queries leave results unchanged.
Missing records return `TFX_NOT_FOUND`; inconsistent/unreadable layouts return
`TFX_INVALID_STATE`; unavailable readers/types return `TFX_QUERY_UNAVAILABLE`.
All clients share a per-dispatch budget of 64 queries, 8,192 memory reads and
1 MiB copied bytes; exhaustion returns `TFX_QUERY_BUDGET_EXHAUSTED`.
Native layouts and type indices are resolved in the current world, then discarded
at callback dispatch exit. Reads support contiguous and shared 32-entry pages.
They copy memory without calling engine component getters or triggering asserts.

Scanning is not a stable snapshot across callbacks/worlds. Re-read returned IDs
in the current callback; never infer a vehicle reservation or continued waiting
from an earlier result. Configured terminals and maximum-load fractions do not
prove actual platform access, free vehicle capacity, destination reachability or
unloading eligibility. Alternative terminals are reported only as present/absent.
Queries may describe assigned passengers and multi-stop legs; policy must still
reject unsupported cases. No result authorizes mutation.

## Vehicle queries (ABI 3)

Initialize `tfx_api_v3 api = {.v2.base.struct_size = sizeof(api)}` and request
`tfx_get_api(TFX_VEHICLE_ABI_VERSION, &api.v2.base)`. ABI 1/2 retain their layouts
and capability values. ABI 3 adds `TFX_CAP_VEHICLE_QUERIES`:

| Operation | Copied result |
| --- | --- |
| `read_stop_vehicles(context, line, stop, out)` | Engine loading index, up to 16 vehicle IDs; larger results fail with `TFX_LIMIT_REACHED`, never silently truncate |
| `read_vehicle(context, vehicle, out)` | Line/stop, raw state/loading enum, selected arrival station index/terminal and lock flag, user-stop flag, passenger capacity, onboard/reserved counts, remaining places and loading timer |

Arrival station is an **index within the stop's station group**, not a station
entity ID. The selected arrival terminal can differ from the configured terminal.
These fields still do not establish access from a passenger's waiting edge.
Raw enum values are build-specific; clients must not treat a nonzero state as
permission to board. Loading timers may be negative; NaN/infinity are rejected.

Remaining places subtract onboard passengers and passengers already assigned in
the terminal system's reverse index. Freight assignments do not count as people.
Stale assignments, duplicate entries, unreadable/malformed tables or occupancy
above capacity fail the query. Empty valid indexes give zero counts. Tables are
bounded to 4,095 slots and reservation sets to 1,024 entries, within the existing
shared read budget. Results describe the instant **before** original selection;
new reservations can be added immediately afterward. No seat is reserved by a
read. Actual platform access, unloading and journey continuation remain required
before any interlining mutation.

## Interception and compatibility

Pinned executable SHA-256:
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
The terminal-system table is at RVA `0x3702758`; slot `+0x58` at `0x37027b0`
points to Update at `0xb0db00`. Static RTTI identifies
`ecs::SimEntityAtTerminalSystem`. Adjacent lifecycle entries and 16 Update bytes
are checked in the relocated image. An atomic expected-pointer exchange changes
the slot; original page protection is restored. A named per-process ownership
handle rejects another cooperating bridge. Independent patchers remain outside
this agreement and can still conflict.

[`entry.S`](entry.S) saves volatile integer/XMM registers and the original stack,
calls the observer, then tail-jumps to the original function. It has Windows x64
unwind metadata and does not relocate engine instructions. The observer checks
the system table, update-data pointer, thread, and modification-window flags.
The original engine call still executes on rejected observations. Only a null
window and enabled nested/post flag are accepted; no engine mutation is called.

Registry tests exercise two consumers, expired tokens, thread/reentrancy guards,
limits, and duplicate IDs. The slot/assembly test checks ownership refusal,
restored page protection, integer/float/stack arguments, original return value,
and exactly one original call. Query tests cover both storage layouts, malformed
and absent records, disabled loading, non-finite fractions, scan pagination,
budgets, output preservation, ABI 2/3 context/thread rules, vehicle capacities,
mixed passenger/freight reservations, index corruption and overflow refusal.
These tests do not prove arbitrary native-client
safety, cross-line migration, save compatibility, or third-party mod support.
See [the query capture](../docs/native-query-validation-2026-10-07.md) for runtime
evidence and the unresolved Proton stop-loader stall. See
[vehicle capture](../docs/native-vehicle-validation-2026-10-07.md) for ABI 3 results
and the remaining live nonzero-reservation/platform-access checks.

Implementation references: Microsoft's [x64 calling convention](https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention),
[atomic pointer exchange](https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedcompareexchangepointer),
and [DLL initialization guidance](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices);
official [LLVM-MinGW release](https://github.com/mstorsjo/llvm-mingw/releases/tag/20250910).
