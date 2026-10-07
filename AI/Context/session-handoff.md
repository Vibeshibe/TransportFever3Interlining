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

1. Continue the native terminal queue/vehicle selection trace. Revision 2 works;
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
