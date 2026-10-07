# Session handoff — 2026-10-07

## Current result

Task 003 remains active. Static analysis confirms the pathfinding hook exchanges
only a town cost-factor map; it cannot itself commit alternate passenger
boarding. [Full evidence](../../docs/scripting-hook-investigation.md) records
the executable fingerprint, function addresses, and unresolved questions.
Task 004 is active; task 005 (shared extender) remains conditional.

The probe loaded and all four events fired in the user's first test. See
[Test 1](../../docs/test-run-2026-10-07.md) for evidence, UTC/local time mapping,
counts, and the 13:52 S2-waiting/S1-arrival observation. Raw log, extract, and
resulting save are preserved under ignored `analysis/test-runs/2026-10-07-test1/`.
Revision 1 runtime files matched the repository at
`game/installation/mods/tpf3_interlining_probe_1/`. Originals were only read.

## Latest implementation

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
for engine mutation, and task 005 remains conditional.

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

1. Target terminal-update entry/exit relative to engine modification and replica
   replay with a narrower observer. Capture 2's generic remove breakpoint consumed
   5,001 hits in about 11 seconds for only nine relevant records. Decode layouts
   passed four unit tests and live observation. Do not reuse saved world pointers:
   both instances participate in updates. The
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
`rtk`. No MCP or native extender is installed.
