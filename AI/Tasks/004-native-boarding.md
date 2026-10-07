# 004 — Trace native boarding and state transitions

- Status: In progress — native queue/arrival functions located
- Owner: Assistant
- Dependencies: 003 identifies a capability missing from scripting

## Objective and output

Find and verify the smallest native intervention that fills the established gap.
Document the relevant call flow, data ownership, and state transitions. Mark this
task skipped if existing scripting is sufficient.

## Acceptance criteria

- [x] Trace diagnostic-string references to actual candidate functions; separate
      string addresses from function addresses and confirm executable version.
- [ ] Establish how line, direction, stop occurrence, platform access, capacity,
      loading rules, and onward journeys affect boarding eligibility.
- [ ] Identify how queue membership, passenger state, manifests, and seat
      allocation are updated together through engine transitions.
- [ ] Investigate thread ownership, callback timing, invalidation, and object lifetime.
- [ ] Define a minimal interception proposal with original-behavior fallback.
- [ ] Verify the candidate through runtime observation on a disposable save
      before calling it a supported implementation point.

## Validation and open questions

Start with the anchors in [research notes](../../docs/interlining-research.md).
The [scripting assessment](../../docs/scripting-hook-investigation.md) establishes
that `OnModifyPathfinding` only exchanges town cost factors. This justifies
tracing native boarding, without yet deciding to implement an extender.
Initial functions and call relationships are documented in the
[native trace](../../docs/native-boarding-investigation.md). Terminal update
uses a two-key vehicle lookup and cargo/loading checks. The done-waiting lead
calls a private-driving helper, not an established transit-boarding transition.
No safe native intervention is verified. Revision 2 diagnostics supplied a same-terminal
S1/S3 reference case in [Test 2](../../docs/test-run-2026-10-07-2.md). Use that
case to investigate vehicle selection and eventual engine-owned transitions.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/004-native-boarding.md`.
