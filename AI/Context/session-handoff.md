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

**Next:** implement arriving-vehicle/capacity/reservation and actual platform
compatibility queries. Configured stops and maximum-load fractions are already
available but insufficient. Then finish candidate revalidation and implement the
one-passenger engine-owned replacement. The sampled person `114323` has S3 stops
1→4, so it does not meet the current adjacent-only offline policy. Keep the original
Test 2 same-terminal adjacent S1/S3 case as the reference; do not switch to an
ineligible multi-stop passenger merely because the scanner found one.

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

1. Finish live vehicle capacity/reservations and actual platform compatibility
   queries; waiting-leg and configured route snapshots are implemented. Then
   implement one-passenger mutation after candidate validation is complete.
   The dedicated scope must finish before original selection starts; inserting
   into the engine's later commit scope would invalidate prior selections.
   Use the current callback's world and preserve the original ABI/call. Loader
   and interception passed the pilot; mutation bindings, live candidate validation
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
