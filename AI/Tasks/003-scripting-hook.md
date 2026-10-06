# 003 — Establish scripting-hook capabilities

- Status: Ready — not started
- Owner: Assistant
- Dependencies: 001; a runtime probe needs 002

## Objective and output

Determine whether existing scripting can support suitable-service boarding.
Produce a capability assessment with source/offset evidence, a minimal probe
if practical, and an explicit script-only versus native-extension decision.

## Work and acceptance criteria

- [ ] Reconfirm the executable fingerprint; preserve analysis settings and logs.
- [ ] Inspect the shipped `OnModifyPathfinding` handler and trace its native
      producer/consumer as necessary using Ghidra cross-references.
- [ ] Establish payload fields, return handling, invocation timing, and whether
      changes apply per town, per passenger, per journey, or another scope.
- [ ] Establish whether a waiting passenger can reconsider eligible lines and
      whether the engine can commit the replacement journey safely.
- [ ] Document any available subscription, ordering, or error-handling semantics.
- [ ] Label runtime-unverified conclusions and supply a probe/test procedure.
- [ ] Record what is sufficient, what remains unknown, and any precise API gap
      before activating native-extension tasks.

## Validation and open questions

Known: the bundled handler uses `town2lineCostFactor`; engine component access
is documented as read-only. Neither fact proves the full event's capabilities.
Ghidra startup works; executable import and decompilation have not been performed.
See [the initial evidence](../../docs/interlining-research.md).

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/003-scripting-hook.md`.
