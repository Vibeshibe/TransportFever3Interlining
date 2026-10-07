# 003 — Establish scripting-hook capabilities

- Status: In progress — probe runtime verified; boarding API investigation remains
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

The [probe](../../probes/pathfinding/README.md) passes offline checks and now
loads in-game: all four events were observed in [Test 1](../../docs/test-run-2026-10-07.md).
Pathfinding reached counter 800, with every logged town map empty. Invocation
intervals usually span two simulation updates in the early samples, with a
longer interval also observed. Native producer identity, complete ordering/error
semantics, and changes to existing journeys remain open.

Revision 2 snapshots worked in [Test 2](../../docs/test-run-2026-10-07-2.md),
correlating passenger `75625` with an earlier same-terminal S3 and later S1.
Next: continue task 004 from this concrete case.
Extender implementation remains conditional.
Milestone history: `git log -- AI/Tasks/003-scripting-hook.md`.
