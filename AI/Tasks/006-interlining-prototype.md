# 006 — Implement and validate alternative boarding

- Status: In progress — native read-only candidate observed; mutation pending
- Owner: Assistant implements; user tests in-game
- Dependencies: 002 baseline and 003 capability decision; 004–005 if required

## Objective and output

Produce a bounded prototype: a passenger may board another suitable line that
reaches their existing planned alighting stop from an accessible boarding
location, while preserving a valid onward journey. Define a consistent initial
decision rule before introducing preference variation.

## Acceptance criteria

- [ ] Determine eligible service/stop occurrences with correct direction,
      platform access, capacity, and load/unload permissions.
- [ ] Compare boarding now with waiting using available journey estimates;
      document estimation limits and fallback behavior.
- [ ] Update through verified engine transitions and preserve onward travel.
- [ ] Account for line edits/removal and invalidate cached alternatives correctly.
- [ ] Provide diagnostics explaining eligibility and boarding decisions.
- [ ] Compare against the baseline for shared-stop and branch-destination trips,
      full vehicles, opposite directions, platform differences, express services,
      transfers, line edits, and save/reload.
- [ ] Check for stuck or duplicated passengers, inconsistent occupancy, and lost
      journeys; record waiting time, travel time, boardings, and simulation cost.
- [ ] Document installation, disabling, known limits, and repeatable test steps.

## Validation and open questions

An [offline candidate module](../../src/interlining/README.md) now accepts the
recorded S1/S3 adjacent segment and rejects 25 unsuitable/invalid controls.
It reads copied snapshots, explains rejection reasons, and requires native
validation for every proposal. It does not rank services, reserve seats, or
change boarding; no acceptance criterion above is complete yet.

The native `it_validate_candidate` now consumes ABI 5 live copied queries. It
requires direct platform access, matching adjacent endpoints, locked ready loading,
terminal load/unload flags, unrestricted line loading, destination presence and
capacity after reservations. Query failures preserve output; exclusions carry
reasons. Fixtures pass and the live capture identified an S1→S3 proposal; see
[evidence](../../docs/native-candidate-validation-2026-10-07.md). This does not
complete the broader criteria: onward travel and actual replacement are untested,
while multi-stop/loop/alternative-terminal cases are deliberately excluded.

Use [the test-save guide](../../docs/test-save.md). Define
performance limits using measured baseline behavior, and avoid full-network
per-passenger recalculation every frame. Broader route replanning is a later
extension if the bounded prototype establishes correctness.

Implementation history: `git log -- src/interlining tests/test_candidate.lua`.
Planning history: `git log -- AI/Tasks/006-interlining-prototype.md`.
