# 003 — Establish scripting-hook capabilities

- Status: In progress — static assessment recorded; runtime probe ready
- Owner: Assistant
- Dependencies: 001; a runtime probe needs 002

## Objective and output

Determine whether existing scripting can support suitable-service boarding.
Produce a capability assessment with source/offset evidence, a minimal probe
if practical, and an explicit script-only versus native-extension decision.

## Work and acceptance criteria

- [x] Reconfirm the executable fingerprint; preserve analysis settings and logs.
- [ ] Inspect the shipped `OnModifyPathfinding` handler and trace its native
      producer/consumer as necessary using Ghidra cross-references.
- [ ] Establish payload fields, return handling, invocation timing, and whether
      changes apply per town, per passenger, per journey, or another scope.
- [ ] Establish whether a waiting passenger can reconsider eligible lines and
      whether the engine can commit the replacement journey safely.
- [ ] Document any available subscription, ordering, or error-handling semantics.
- [x] Label runtime-unverified conclusions and supply a probe/test procedure.
- [x] Record what is sufficient, what remains unknown, and any precise API gap
      before activating native-extension tasks.

## Validation and open questions

The native bridge constructs and reads back only `town2lineCostFactor`.
This hook alone cannot commit alternative boarding. The dispatcher processes
successful non-nil returns; the passive probe returns nil without mutating the
payload. See [the assessment](../../docs/scripting-hook-investigation.md) for
function addresses, evidence limits, and the precise gap.

Ghidra import and targeted decompilation succeeded; whole-program analysis did
not complete. Producer cadence, downstream application to existing journeys,
full error handling, and mod ordering remain unresolved. A follow-up candidate
at `0x140b35820` did not establish signal emission timing; do not treat its
stack-relative `+0x170` access as the system signal.

The [probe](../../probes/pathfinding/README.md) passes offline neutrality and
sampling checks, Lua syntax checks, manifest/resource validation, and archive
CRC validation. It is packaged locally, not installed or game-validated. The
user agreed on 2026-10-07 to run the initial disposable-save test.

Next: verify initialization and capture actual events; independently follow
native boarding under task 004. Extender implementation remains conditional.
Milestone history: `git log -- AI/Tasks/003-scripting-hook.md`.
