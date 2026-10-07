# Session handoff — 2026-10-07

## Current result

Task 003 remains active. Static analysis confirms the pathfinding hook exchanges
only a town cost-factor map; it cannot itself commit alternate passenger
boarding. [Full evidence](../../docs/scripting-hook-investigation.md) records
the executable fingerprint, function addresses, and unresolved questions.
Task 004 is active; task 005 now has an implemented and runtime-observed read-only
native bridge. Passenger mutation remains unimplemented.

The probe loaded and all four events fired in the user's first test. See
[Test 1](../../docs/test-run-2026-10-07.md) for evidence, UTC/local time mapping,
counts, and the 13:52 S2-waiting/S1-arrival observation. Raw log, extract, and
resulting save are preserved under ignored `analysis/test-runs/2026-10-07-test1/`.
Revision 1 runtime files matched the repository at
`game/installation/mods/tpf3_interlining_probe_1/`. Originals were only read.

## Latest implementation

Bridge **0.5 / ABI 5** adds terminal passenger load/unload flags and entity presence.
ABI 1–4 remain compatible. `src/interlining/native_candidate.c` implements the
restricted adjacent-leg validator entirely through the public SDK. No mutation
operation is exposed. See [candidate evidence](../../docs/native-candidate-validation-2026-10-07.md)
for static layouts, test commands, hashes and runtime results.

All three suites (`query_tests`, `bridge_tests`, `candidate_tests`) passed under
the isolated Wine prefix, following a warnings-as-errors cross-build. User restarted,
loaded a disposable baseline paused, and unpaused after the observer loaded.
`analysis/native-bridge/build-v5/tpf_shared_bridge.dll` loaded into Windows PID 308;
log `tpf-bridge-308-29658466.log`, thread 596, worlds `0x2c5d3aa0` / `0x2c5d4760`.
Capture completed 1,200 callbacks: 15 remembered people, 11 vehicles, 45 ready
samples, 27 validations: 7 positives (same observed pair), 12 unsupported legs,
8 boarding mismatches. Zero query errors/rejected boundaries. Observer auto-disabled;
no debugger or stop-loader used. User was told they can pause/close. Restart
before a revised DLL; do not overwrite the loaded build or assume its PID.

First positive proposal: sequence 47, person **22747**, S1 `98278`, stops **1→2**,
destination **86002**, S3 `97743` vehicle **97959**, stops **1→2**. Boarding station
**97948** terminal **1**, alighting station **97294** terminal **0**, places left
**40**. All implemented gates passed in that callback. No passenger was moved.
Multiple-stop, terminal-mismatch and loop-wrap cases were also rejected.

New layouts: Station RTTI RVA `0x3cea860`, stride `0x58`, first vector terminals
stride `0x88`; `passengersLoad +0x61`, `passengersUnload +0x63`. Evidence getter
`0x1405d9640`, registration `0x1422c1ab0`, export `terminal-permissions.txt`.
Entity presence requires a nonempty component row consistent with the world mask;
it does not prove reachability. Vehicle gate state2/load3, locked configured
arrival, finite timer<=0. Load mode0/maxLoad1.0 only; fractional compartment
allocation unresolved. No alternatives, multi-stop legs, wrap or node fallback.

**Next:** implement a one-passenger engine-owned waiting replacement with current
expected-state revalidation, then capture boarding/alighting/continuation and
replication. Preserve native arrival bytes and unassigned vehicle -1. Dedicated
modification scope ends before original terminal selection. No behavior fix,
mutation API, Lua binding or stable release exists yet. Do not repeat generic
query/platform captures; a positive read-only candidate is now observed.

### Earlier platform implementation

Bridge **0.4 / ABI 4** now exposes `read_person_platforms`: copied direct edge
membership as station-entity/terminal pairs, plus edge entity/index and place.
ABI 1–3 remain compatible. **Platform membership and a nonzero reservation are
now observed live**; do not repeat generic captures to establish those facts.
See [platform/reservation evidence](../../docs/native-platform-validation-2026-10-07.md).

The user restarted the disposable save. Loading `analysis/native-bridge/build-v4/`
returned zero into Windows PID 308. Capture `tpf-bridge-308-27811482.log` completed
1,200 callbacks, simulation thread 592, worlds `0x2cd5ea30` / `0x2cd5f6f0`.
Nine platform snapshots, 15 Wimborne comparisons, 64 vehicle records across ten
IDs, four nonzero-reservation vehicle samples, one assignment cross-check, no
logged query errors and zero rejected boundaries. Observer auto-disabled; no
GDB attachment or stop-loader. User was told they can pause/close. Restart before
loading a revised DLL; do not overwrite the loaded artifact or assume its PID.

At sequence 595, person **71583** was assigned to vehicle **97913**, S3 stops 4→5;
vehicle API reported capacity 40, onboard 0, reserved 1, remaining 39. This
corroborates that sampled reservation, not an exhaustive census. At Wimborne,
S1 persons 98760/113752 and S3 person 114323 map to station 97948 terminal 1,
while S2 person 75514 maps to terminal 2. S1/S3 share physical access; the subsequent ABI 5 consumer validates route and
alighting compatibility within the current callback.

Current-world discovery: world `+0x30` vector of 8-byte system pointers (dispatcher
`0x142bb8a50`); find unique StationSystem vtable RVA **0x3706600** (RTTI RVA
0x3d972a0, locator VA 0x14390eee8). System `+0x38` dynamic object, map `+0x70`,
8-byte edge keys, 32-byte entries, vector at entry `+8`, 8-byte station-entity /
terminal pairs. `0x140b3fe60` insertion proves meanings and permits many terminals
per edge; station-wide person edges map to every terminal. Place registration
`0x14195ec60` proves edge at0/place at8, stride12. Exports `platform-system.txt`
and `platform-place-layout.txt` supplement the prior binding chain below.

Native helper **0x140952760** tries direct edge membership, then endpoint-node
membership using **0x140b41150**. ABI 4 does not implement that fallback. Initial
candidate validation can reject empty direct membership instead of guessing.
Reader caps: system vector256, map4095 slots,16 returned pairs, existing shared
64-query/8192-read/1MiB budgets. Tests passed, including two distinct worlds,
shared storage, missing/ambiguous system, malformed data and ABI4 thread/token.

This platform milestone led to the ABI 5 candidate validator above. Native
infrastructure remains separate from interlining policy.

### Earlier vehicle implementation

Bridge 0.3 adds ABI 3 `read_stop_vehicles` and `read_vehicle`, with capacity,
onboard passengers, assigned boarding reservations, remaining places, raw state,
loading timer, selected arrival terminal and locked/user-stopped flags. ABI 1/2
remain compatible. See [vehicle evidence](../../docs/native-vehicle-validation-2026-10-07.md)
for layouts, tests and hashes. Selected `arrival_station_index` is an index in the
station group, **not an entity ID**. No platform-access or mutation API yet.

Build `analysis/native-bridge/build-v3/` was loaded successfully into the user's
restarted disposable save, Windows PID 312. Log `tpf-bridge-312-26298912.log`:
400 accepted callbacks, thread 592, alternating worlds `0x37dc2200`/`0x37dc2ec0`,
10 vehicle snapshots across five S1/S2/S3 vehicles, 15 passenger records and 30
stop records; no sampled query errors or boundary rejections. Auto-disabled;
no debugger or optional stop-loader was used. User was told they can pause.
Restart before loading a revised DLL. Do not assume the process is still alive.

All sampled vehicles had capacity 40 and zero reservations. Onboard changes were
observed. Nonzero/mixed/stale reservation tests passed fixtures but still need
live independent validation. Both native suites passed; final source rebuilt in
`build-v3-final/` after correcting the outdated version log string (loaded v3
still prints 0.2/ABI1,2). No query/hook implementation changed after capture.

The test consumer cycles through baseline line IDs 98278/97742/97743 and their
stops; SDK queries remain generic. Keep this save-specific research consumer
separate from future interlining policy. Remaining places are pre-selection
snapshots, never seat reservations.

**Next:** finish actual waiting-edge terminal membership and nonzero-reservation
runtime validation, then full candidate checks and the engine-owned one-passenger
experiment. No further generic boarding trace is needed.

Platform lookup traced through Lua binding: string `0x14373fdb8` in registration
`0x140f27a80` -> binder `0x140eae4c0` -> closure builder `0x140ee5fb0` -> thunk
`0x140f02e00` -> invocation `0x140ef92b0` -> native getter **`0x140b41050`**.
The invocation obtains a StationSystem pointer from returned engine context
`+0x158`; native getter reads StationSystem dynamic pointer `+0x38`, then edge-key
map `+0x70` (key 8 bytes, lookup `0x14093e4e0`, entry stride `0x20`, vector at
entry `+8`). Return vector entries appear 8 bytes. Verify their field meanings,
waiting-place EdgeId layout and a **current-world** route to this system before
implementing the reader. Do not blindly reuse the scripting engine context.
Exports `platform-bindings.txt`, `platform-callable.txt`, `platform-dispatch.txt`,
`platform-method.txt`, `platform-invoke.txt`, `platform-native.txt` retain the chain.
Vehicle exports `vehicle-readers.txt` and `platform-readers.txt` establish the
other layouts; the evidence doc lists native addresses and offsets.

### Earlier passenger/route implementation

Bridge 0.2 now implements and runtime-verifies copied waiting-person and line-stop
queries. See [query validation](../../docs/native-query-validation-2026-10-07.md).
ABI 2 adds `read_waiting_leg`, `read_line_stop`, `scan_waiting_people`; ABI 1 remains
compatible. Private readers resolve types per world/dispatch and copy contiguous
or shared-page components, with explicit failures and bounded reads. No game
component getters or mutation operations are called. Passenger snapshots include
destination, original arrival bits, and optional opaque terminal-place bytes.
Stop snapshots resolve configured station/terminal and passenger loading settings.

Builds are in `analysis/native-bridge/build-v2/` (do not overwrite a loaded DLL).
Both `bridge_tests.exe` and `query_tests.exe` passed. The restarted disposable game
was Linux PID 87399 / Windows PID 308. Capture
`tpf-bridge-308-23938409.log` logged 20 passenger records (8 IDs), 40 stop records,
400 callbacks, zero query errors and zero boundary rejections. Windows thread 596;
worlds `0x2eb2bca0` and `0x2eb2c960`. S3 uses Wimborne terminal 1, S2 terminal 2.
This capture includes assigned configuration/data reads, not an eligibility proof;
no interlining behavior change is active.

Observer auto-disabled. A subsequent optional `--stop` loader hung without a result.
Directly read DLL counters: `enabled=0`, `observed=400`, `rejected=0`. Terminated
only the hung loader (Linux PID 89794); no game signal or injected-thread kill.
No debugger is attached to the game. Loader-only GDB inspection was denied by OS
ptrace permissions. Stop-stall cause is unresolved; wrap future Wine loader calls
in `rtk proxy timeout 45 ...` because its internal timeout covers remote-thread
waits, not Wine startup/all APIs. User can pause/close; restart before a new DLL.

The earlier sampled person `114323` has S3 stops 1→4, so it does not meet the
current adjacent-only offline policy. Keep the original Test 2 same-terminal
adjacent S1/S3 case as the reference.

### Earlier bridge implementation

The shared native bridge is implemented in `native/` and passed an in-game
400-invocation read-only capture. See [validation](../../docs/native-bridge-validation-2026-10-07.md)
and [build/API/loader guide](../../native/README.md). It replaces the terminal
virtual slot at RVA `0x37027b0` (table `0x3702758`, offset `+0x58`) after full
executable hash and live code/table checks. An assembly wrapper preserves input
registers/stack and tail-jumps to original Update. No instruction relocation or
engine mutation calls. ABI 1 has registration, unregistration and copied
observations, with scoped tokens, deterministic order and 32-client limit.

LLVM-MinGW 20250910 was downloaded, hash-verified and extracted into ignored
`tools/vendor/`. `rtk proxy python3 tools/build_native_bridge.py` builds into
`analysis/native-bridge/build/`; do not overwrite the DLL if still loaded.
Isolated Windows tests passed, including two consumers, thread/token/limit guards,
slot ownership/protection and original argument/return preservation. The loader
test correctly rejected an unsupported fixture executable.

The user's running disposable game was Linux PID 79540 / Windows PID 308, using
GE-Proton11-5 and prefix `/home/vibeshibe/Games/Heroic/Prefixes/default/Transport Fever 3/pfx/`.
Loading returned zero. After the user unpaused, 400 calls were accepted; sampled
records show thread 592, two alternating worlds, and zero rejected boundaries or
dispatch errors. Observation auto-disabled and explicit `--stop` returned zero.
The game has no debugger attached. The DLL remains pinned, forwarding only,
until game exit; no autoload, disk game changes or passenger changes. It is safe
for the user to pause/close. The installed passive Lua probe is unchanged.

This established the interception for the queries above. Do not repeat generic
boarding or boundary captures. No behavior fix is active.

### Earlier boundary evidence

Terminal Update entry is now the selected intervention candidate. Two boundary
captures on the reopened disposable save paired 280 complete calls. Entry and
return always had no modification window and nested/post flag one; worlds
alternated. No commit-scope interior site fired. Person-system line-index removal
was also traced and asserts one removal, complementing the insertion callback.
See [the adapter design](../../docs/native-adapter-design.md) and
[shared-extension draft](../../docs/shared-extender-design.md).

The observer now has `boundary` mode with per-thread stack/world/phase checks.
Eight decoder/correlation unit tests pass. Both captures detached and the game
reported `TracerPid: 0`; no mutation or game-file changes occurred. This informed
the subsequent read-only bridge implementation above. Cross-line writes remain
unverified; no interlining fix is active.

Native runtime observation now succeeded with a new
[GDB observer](../../tools/native_trace/README.md). It checks the pinned executable
and live code, uses hardware breakpoints, and logs copied component/stack data.
The user unpaused a disposable save. Capture 2 recorded person `71583` waiting
for S3, approaching vehicle `97914`, and entering the ordinary onboard transition.
All nine records ran on simulation thread `75558`, with two engine worlds and
replica replay call sites. The debugger detached; no engine calls or state writes
were made. See [evidence](../../docs/native-runtime-observation-2026-10-07.md).

Static follow-up found same-line stop remapping uses remove/re-add, preserving
arrival time. Engine mutation requires an active modification window. `Post`
requires an additional flag; it is not a general cross-thread dispatcher.
Cross-line migration and a write-capable insertion point remain unverified.

Follow-up native analysis mapped line/stop vehicle selection, its exact vehicle
assignment store, separate vehicle/queue indexes, and the common onboard
transition. See [the trace](../../docs/native-boarding-investigation.md).
`src/interlining/candidate.lua` now implements an offline-only proposal policy;
`rtk proxy lua tests/test_candidate.lua` passes the reconstructed Test 2 case
and 25 rejection controls. No runtime adapter or behavior-changing mod exists.
Task 006 is in progress for this isolated policy; task 004 remains unresolved
for engine mutation, and task 005 has a design draft.

Revision 2 of the passive probe is implemented, tested offline, packaged, and
installed in the existing mod directory. The game was not running during the
update. Two changed files were checked against revision 1 and backed up in
`analysis/probe-install-backups/revision1-to-2/` before replacement. No shipped
resources, executable, or save files were changed. New arrival queries are runtime-verified in
[Test 2](../../docs/test-run-2026-10-07-2.md); no interlining behavior fix is active.

The probe logs other lines' waiting queues at the same station group, line and
station names, planned alighting stations, actual/configured terminals, and
capacity/occupancy. Limits: 16 queues, 8 people per queue, 32 people per arrival;
first 60 arrivals then every 100th. See [instructions](../../probes/pathfinding/README.md).
Native investigation found terminal update/vehicle arrival routines; see
[the trace](../../docs/native-boarding-investigation.md). Do not hook the
private-driving helper reached through `NoteSimPersonDoneWaiting` as though it
were a transit-boarding transition.

## Resume checklist

1. Implement the bounded one-passenger engine-owned replacement. The read-only
   adjacent candidate, direct platform membership and nonzero reservations have
   live evidence; ABI 5 terminal/presence queries are implemented.
   The dedicated scope must finish before original selection starts; inserting
   into the engine's later commit scope would invalidate prior selections.
   Use the current callback's world and preserve the original ABI/call. Loader
   and interception passed the pilot; mutation bindings, live onward-journey validation
   and controlled mutation tests are outstanding. Boundary observation and paired
   line-index callbacks are done;
   do not repeat generic boarding captures. The
   vehicle assignment is at `0x140b0f49f`; component-change callbacks
   `0x140b0c730`/`0x140b0c7f0` maintain only the vehicle index. Queue lifecycle
   callbacks are `0x140b0c8b0`/`0x140b0d030`. Common transition `0x140b374c0`
   conditionally adds the seat component and independently adds the journey's
   onboard component, retaining passed line/stop values. Verify a
   safe callback boundary, ABI, reservations and rollback before any writes.
   Revision 2 works;
   another generic diagnostic run is not needed. Test2 logs/save are preserved
   in `analysis/test-runs/2026-10-07-test2/`.
2. Use person `75625`, S1/S3 Wimborne stop 1 to Melksham stop 2, as the reference
   same-terminal case. S3 arrived at 14:19:53 with 37 spare places, person stayed
   in the S1 queue, S1 arrived 14:21:47, and S1 line-use started 14:21:52.
   S2 uses a different terminal. Preserve user-reported S2/time details separately
   from the logged S3 case; see Test 2 for correlation limits.
3. Finish task 003's cadence, result-ordering/error, and downstream-use questions.
   `OnStartedLineUsage` does not prove the person has boarded.
4. For native investigation, trace engine-owned boarding and journey transitions;
   never substitute writes to a line field for an atomic state transition.
   Keep any eventual extender reusable and separate from interlining policy.

## Local analysis state

Ghidra 12.1.3 project: `analysis/ghidra/TPF3Interlining`; raw evidence:
`analysis/hook-investigation/`. Both are ignored. Whole-program analysis failed
partially and timed out; targeted scripts work. Do not rerun import unnecessarily.
Use [the export instructions](../../tools/ghidra/README.md).

Native follow-up exports are under ignored `analysis/native-boarding/`, including
`queue-maintenance.txt`, `queue-lifecycle.txt`, `boarding-transitions.txt`,
`boarding-component-and-binding.txt`, and `terminal-update.asm`. The helper
`tools/native_references.py` uses locally available `pefile` 2024.8.26 to find
diagnostic LEA candidates. Exception regions may be cold fragments, not entries.
Ghidra's approach-helper return type is wrong; do not copy inferred ABIs blindly.

Latest exports: `line-change-transitions.txt` (`0x140b307b0`, same-line remap),
`queue-migration.txt`, `person-component-lifecycle.txt`, `entity-notifications.txt`,
and `engine-modification.txt`. Engine addresses: BeginModification `0x142bb4640`,
EndModification `0x142bb4d90`, Post `0x142bb6ea0`, Replicate `0x142bb78f0`,
replay `0x142bb4430`. `0x142bb4453` is only a replay fragment.
Live captures: ignored `analysis/native-boarding/runtime-trace-01.jsonl` (paused,
empty), `runtime-trace-02.jsonl` (successful), and `runtime-threads.txt`.
New captures: `runtime-boundary-01.jsonl` (80 calls) and
`runtime-boundary-02.jsonl` (200 calls); details/hashes in the runtime evidence doc.
New query exports: `read-lookups.txt`, `read-storage.txt`, `read-layouts.txt`.
Registry helper `0x140099970`; handle lookup `0x1400a4b90`; typed accessors
`0x140095a50` (32 bytes), `0x140095bc0` (152), `0x140095970` (24),
`0x1400955b0` (40, receives world+0x48), `0x1402809d0` (12).
Line/stop configuration registration `0x1418829c0` exposes field offsets.
Native query code and evidence document the actual implemented layouts.

Earlier exports: `terminal-scope.txt`, `person-index-removal.txt`, and
`system-update-dispatch.txt`. Dispatcher `0x142bb8a50` calls Update through vtable
slot `+0x58` (return `0x142bb8aec`). Scoped wrapper `0x142bbbba0` opens changes;
`0x142bbc060` closes them. Terminal interior sites `0x140b0f40e`/`0x140b0f5ef`
are not runtime-validated. Person-index removal callback `0x140b2c450` calls
`0x140953820`; insertion `0x140951180` is its counterpart.
GDB attachment worked under reviewed shell escalation without system ptrace
configuration changes. The Proton process is named `Main Thread`, so identify
it by executable command line, not a process-name search for TransportFever.

Callback bridge `0x140c390e0`, decoder `0x140c35f60`, dispatcher `0x140f43c40`,
subscriber iteration `0x140aae550`. Registration `0x140b34a00` connects the
modifier signal at system offset `0x170`. Its emitter remains unidentified.
`callback-timing.txt` exports `0x140b35820` and `0x140f40f80`; these did not settle
cadence or full error semantics. `person-system-disassembly.txt` is a local
candidate-search artifact, not a validated function map.

Offline observer tests, Lua syntax checks, and package validation pass.
Research and implementation stay on `dev`; `main` remains for validated releases.
Find this milestone with `git log -- docs/scripting-hook-investigation.md`.
Git metadata/network access may need reviewed escalation; authenticated pushes
previously worked through the repo's `libsecret` helper. All shell commands use
`rtk`. No Ghidra MCP is installed. The native bridge was loaded explicitly for
this pilot and is not an installed/autoloading mod.
