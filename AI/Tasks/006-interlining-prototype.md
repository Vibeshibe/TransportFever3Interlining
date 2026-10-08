# 006 — Implement and validate alternative boarding

- Status: In progress — one-passenger engine-owned change implemented; first complete alternate-line journey observed
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
- [x] Update through verified engine transitions and preserve onward travel
      for the initial one-passenger adjacent-leg pilot.
- [ ] Account for line edits/removal and invalidate cached alternatives correctly.
- [x] Provide diagnostics explaining eligibility and boarding decisions.
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
change boarding. Actual replacement is supplied by the later ABI 6 implementation
below.

The native `it_validate_candidate` now consumes ABI 5 live copied queries. It
requires direct platform access, matching adjacent endpoints, locked ready loading,
terminal load/unload flags, unrestricted line loading, destination presence and
capacity after reservations. Query failures preserve output; exclusions carry
reasons. Fixtures pass and the live capture identified an S1→S3 proposal; see
[evidence](../../docs/native-candidate-validation-2026-10-07.md). This does not
complete the broader criteria. The later ABI 6 trial below observed replacement
and onward travel for one passenger; multi-stop/loop/alternative-terminal cases
remain deliberately excluded.

Use [the test-save guide](../../docs/test-save.md). Define
performance limits using measured baseline behavior, and avoid full-network
per-passenger recalculation every frame. Broader route replanning is a later
extension if the bounded prototype establishes correctness.

Implementation history: `git log -- src/interlining tests/test_candidate.lua`.
Planning history: `git log -- AI/Tasks/006-interlining-prototype.md`.

ABI 6 implementation and offline guard/operation tests are complete; the first
[live replacement trial](../../docs/native-mutation-validation-2026-10-08.md)
completed a full alternate-line journey. Broader acceptance criteria remain open.

At sequence 2473 in revision 0.6.1, S1 person 72845 changed to S3 vehicle 97914 and
was observed onboard in both engine worlds. One attempt; expected arrival bytes,
destination 72405 and waiting place preserved. Alighting at planned stop 2 and
arrival at destination 72405 were subsequently observed in both worlds, with no
second engine attempt. Revision 0.6.2's rolling discovery correction is built and
regression-tested separately but not loaded. See the linked trial record for exact hashes and limitations.
