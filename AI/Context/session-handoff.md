# Session handoff — 2026-10-07

## Current result

Task 003 remains active. Static analysis confirms the pathfinding hook exchanges
only a town cost-factor map; it cannot itself commit alternate passenger
boarding. [Full evidence](../../docs/scripting-hook-investigation.md) records
the executable fingerprint, function addresses, and unresolved questions.
Task 004 is ready; task 005 (shared extender) remains conditional.

The probe loaded and all four events fired in the user's first test. See
[Test 1](../../docs/test-run-2026-10-07.md) for evidence, UTC/local time mapping,
counts, and the 13:52 S2-waiting/S1-arrival observation. Raw log, extract, and
resulting save are preserved under ignored `analysis/test-runs/2026-10-07-test1/`.
Installed runtime files match the repository at
`game/installation/mods/tpf3_interlining_probe_1/`. Originals were only read.

## Resume checklist

1. Extend read-only diagnostics with line-name mapping and bounded waiting-state
   snapshots. The current log cannot identify the passenger who ignored S1.
2. Complete task 002's station/platform, available capacity, and journey details;
   one user-observed example is now recorded. The preserved baseline hash begins
   `c228e860e25a`.
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
