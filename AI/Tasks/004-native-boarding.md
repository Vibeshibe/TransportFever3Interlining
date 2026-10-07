# 004 — Trace native boarding and state transitions

- Status: In progress — entry boundary selected and observed; cross-line write test pending
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
- [x] Define a minimal interception proposal with original-behavior fallback.
- [ ] Verify the candidate through runtime observation on a disposable save
      before calling it a supported implementation point.

## Validation and open questions

Start with the anchors in [research notes](../../docs/interlining-research.md).
The [scripting assessment](../../docs/scripting-hook-investigation.md) establishes
that `OnModifyPathfinding` only exchanges town cost factors. This justifies
tracing native boarding, without yet deciding to implement an extender.
Functions and call relationships are documented in the
[native trace](../../docs/native-boarding-investigation.md). Terminal update
looks up vehicles by line/stop and assigns them with component notifications.
Those notifications maintain a vehicle index; separate entity lifecycle callbacks
maintain the line/stop queue. The onboard transition copies the original line
and stop indices with the selected vehicle and adds both onboard components.
Neither a line-field write nor a widened vehicle list coordinates all this state.
The done-waiting lead calls a private-driving helper, not a transit transition.
No safe native intervention is verified. Revision 2 diagnostics supplied a same-terminal
S1/S3 reference case in [Test 2](../../docs/test-run-2026-10-07-2.md). Use that
case to investigate vehicle selection and eventual engine-owned transitions.

Next: verify queue migration, onward-journey ownership, callback/thread safety,
seat allocation and failure recovery before a write-capable adapter. The minimal
proposal in the trace is not yet a verified interception point. Task 006's pure
policy can be tested independently, but does not fulfill native acceptance.

Follow-up found `NoteLineChanged` removing/re-adding terminal components for
same-line stop remapping while preserving arrival time. Identified the engine's
modification windows, posted callbacks, and replica replay. A bounded hardware
breakpoint observer captured ordinary S3 boarding on the simulation thread and
matching operations in two engine instances; see
[runtime evidence](../../docs/native-runtime-observation-2026-10-07.md).
Runtime observation validates those arguments and flags, not arbitrary writes.
Next target: terminal-update boundaries relative to modification and replication;
complete line-index removal semantics before selecting a cross-line insertion.

That follow-up is now complete: 280 paired Update calls observed the same
empty-window entry/return conditions, with alternating worlds. Person-system
line-index removal and insertion helpers were traced. The
[adapter proposal](../../docs/native-adapter-design.md) selects Update entry,
with a separate modification scope finished before original candidate selection.
No write-capable hook has been validated; the runtime acceptance item remains
open until a controlled reassignment succeeds. Native bindings/extension work
can proceed against this bounded candidate.

Research/tooling history: `git log -- tools/native_trace docs/native-boarding-investigation.md`.
Planning history: `git log -- AI/Tasks/004-native-boarding.md`.
