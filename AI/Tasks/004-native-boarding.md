# 004 — Trace native boarding and state transitions

- Status: Conditional — not started
- Owner: Assistant
- Dependencies: 003 identifies a capability missing from scripting

## Objective and output

Find and verify the smallest native intervention that fills the established gap.
Document the relevant call flow, data ownership, and state transitions. Mark this
task skipped if existing scripting is sufficient.

## Acceptance criteria

- [ ] Trace diagnostic-string references to actual candidate functions; separate
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
Diagnostic names are leads, not verified hook locations. No native intervention
has been identified or validated yet; blockers and uncertainty must be recorded.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/004-native-boarding.md`.
