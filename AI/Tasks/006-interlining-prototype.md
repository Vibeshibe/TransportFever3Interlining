# 006 — Implement and validate alternative boarding

- Status: Pending
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

No prototype exists. Use [the test-save guide](../../docs/test-save.md). Define
performance limits using measured baseline behavior, and avoid full-network
per-passenger recalculation every frame. Broader route replanning is a later
extension if the bounded prototype establishes correctness.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/006-interlining-prototype.md`.
