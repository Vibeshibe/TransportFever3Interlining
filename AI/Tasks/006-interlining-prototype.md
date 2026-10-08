# 006 — Implement and validate alternative boarding

- Status: In progress — four batch boardings; five boardings, four original arrivals, live removal classification validated
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
second engine attempt. Revision 0.6.2's rolling discovery correction passed
regression tests and a later short capture without a mutation. Revision 0.6.4
subsequently switched a second passenger and preserved an onboard save checkpoint.
See [the repeat/reload trial](../../docs/save-reload-trial.md) for current evidence.

The [repeat/save-reload trial](../../docs/save-reload-trial.md) is now complete for
person 106847: S1→S3 boarding, saved onboard state restored after restart, native
S2 transfer and original-destination arrival in both worlds. Only read-only
observers ran after restart. This supplies one checkpoint/transfer case, not
blanket save compatibility or measured journey-time improvement. The next
implementation milestone is a small bounded multi-passenger trial with fresh
validation and duplicate-selection protection across worlds.

Version 0.7.0 implements the bounded multi-passenger milestone with an explicit
1–5 person cap (default one), host deduplication/failure latch, and consumer
boarding/arrival tracking. Offline tests pass; live results are tracked in
[the multi-passenger trial](../../docs/multi-passenger-trial.md).

Live version 0.7.0 confirmed four distinct replacements and both-world boardings,
then auto-disabled after eight journey-not-found reads before a fifth attempt.
Read-only diagnostics exposed missing simulation-person state for one entity and
an unmatched at-building ID for another. Preserve bounds; next investigate
lifecycle/identity semantics before another mutation trial. Original destinations
for this batch remain unverified. Checkpoint/evidence are in the trial record;
version 0.7.1 adds diagnostics only and has not been loaded into the game.

The [read-only lifecycle control](../../docs/passenger-lifecycle-investigation.md)
now reproduced both removal120770 and workplace invalidation122261 without
interlining switches. Each coincided with Scaffold appearing on the associated
home/work building. Next implement copied lifecycle diagnostics and regression
fixtures; distinguish removed/invalidated journeys from original arrival while
retaining mutation bounds and current stop behavior. Broader batch validation
and exact native causal tracing are still unproven.

Version0.7.2 implements the copied ABI7 lifecycle query and consumer classifications
with passing query, API contract, mutation and pilot regressions. First anomalies
permanently block new proposals; confirmed removal/invalidation never adds an
arrival. Prepared cap5/18,000-callback build-v7c, not loaded. Next run the documented
[live classification trial](../../docs/lifecycle-tracking.md) from the restored
pre-trial baseline after user confirms restart/load/paused readiness.

The version0.7.2 live trial is complete: five switches/boardings, four original
arrivals and one separately classified both-world removal, zero query errors.
A completed arrival survived later workplace invalidation in the tracker. No
fifth arrival is claimed; removal cause is not attributed. Live result and
coverage limits are in [the lifecycle trial](../../docs/lifecycle-tracking.md).
Next bounded implementation may handle classified lifecycle outcomes explicitly,
keeping unexplained identity/read errors as hard stops; route expansion remains
separate. No active watcher or further running time is needed for this trial.

Version0.7.3 now implements continuation after both-world lifecycle confirmation
for previously confirmed boardings. IDs/attempts remain consumed; unresolved
failures retain their stops. Normal and reserved-attempt fixtures pass. Prepared
build-v7d reserves the fifth attempt to demonstrate a later successful change
following lifecycle resolution. Next perform the [fresh bounded trial](../../docs/lifecycle-continuation.md);
no new game load or injection occurred during implementation.


Latest0.7.3 live trial: four switches/boardings, then home/identity guard stop;
no fifth attempt or confirmed lifecycle resolution. Continuation remains unproven.
Added and fixture-tested precise identity-stop diagnostics, not loaded into game.
See the continuation guide for the paused snapshot, hashes and next evidence gap.

Version0.7.4 prepared and fixture-tested: a cleared home pauses new proposals for
up to32 callbacks awaiting confirmed removal; unknown changes still stop. Not
loaded. See [cleared-home policy](../../docs/home-clear-handling.md). The separate
0.7.3 diagnostic capture remains active; avoid mixing build outcomes.

Diagnostic capture finished at7909:120842 home75531→-1, target78804 retained;
later both-world absence confirmed. Four boardings/three original arrivals; no
fifth or continuation.0.7.4 is prepared (4d6e70a), requires fresh live validation
of its32-callback pending-removal window. All watchers/readers ended.
