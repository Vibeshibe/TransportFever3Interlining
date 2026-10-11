# 006 — Implement and validate alternative boarding

- Status: In progress — 0.9.1 station-focused discovery implemented; offline validation complete, integrated live trial running
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

Latest0.7.4 live result: home_clear_timeout at2013 despite matching clear state
at1981/1982; passenger still onboard over a minute later, then removed. Three
boardings/no original arrivals observed.0.7.5 now classifies matching both-world
home loss separately; fixture/host/contract tests pass, build-v7f NOT loaded.
See [home invalidation](../../docs/home-invalidation.md). No active watchers.

Completed0.7.5 live trial: five boardings, four original arrivals and one removal;
after confirmed removal7190, the reserved fifth passenger switched9214,
boarded9247 and arrived13695. All outcomes observed, zero query errors, audit
passed; all watchers ended. **Continuation after removal is live-validated;
home_invalidated remains fixture-only because no home-loss state was captured.**
See [complete evidence](../../docs/home-invalidation.md). Next implement bounded
shared-route eligibility beyond the save-specific adjacent pair, preserving the
planned alighting stop and existing access/capacity/revalidation/lifecycle guards.
No additional replay or save load is needed to finish this completed trial.

Version 0.8.0 implements demand-driven line discovery and one-to-four-interval
shared legs, with ordered express skips and exact copied-route revalidation.
No hard-coded baseline line IDs or stops1→2 gate remain. Nine automated test
invocations pass; worst consumer allowance 62/64. The ABI, native mutation path,
attempt/lifecycle and platform/capacity guards remain. Build-v8a is NOT loaded.
See [implementation and live test](../../docs/route-expansion.md). Multi-stop/express
live coverage, extra-stop local substitutes, alternate platforms, preference/time
comparison and broad edited-line/save validation remain open.

0.8.0 live trial stopped on eight waiting-reader errors after one completed
adjacent journey. Paused records expose native-3 vehicle sentinel; frozen cursor
identifies the last failed candidate. Prepared0.8.1 copies known-2/-3 but excludes
both from mutation, retaining unknown-state stops. All nine regressions pass;
no active observers and new DLL not loaded. Multi-stop/express coverage is still
pending. [Sentinel evidence and correction](../../docs/waiting-sentinels.md).

0.8.1 live checkpoint9600: one adjacent S3→S1 switch/boarding and original
destination arrival, zero query errors. Sentinel exclusions and multi-stop/express
boarding have not appeared. Capture continues under its original5/18000 bounds.
The [next capacity plan](../../docs/passenger-capacity-plan.md) separates an attempt
ledger, one boarding confirmation and four detailed journey samples. It is a
proposal, not a higher-capacity implementation or authorization to change the
running trial's bounds.

Completed0.8.1 capture: callback_limit18000, three adjacent switches/boardings,
two original arrivals, third last observed on native onward S3. Zero query errors
or special waiting samples; multi-stop/express boarding unexercised. Audit passes,
all watchers ended, no further replay needed. Full evidence in the sentinel guide.
Next implement the capacity plan with bounded discovery metrics and sampled
journey diagnostics; keep fresh per-switch checks and explicit coverage gaps.

Version0.9.0 implements32-attempt capacity with four concurrent journey samples
and one pending boarding. Released trackers never release IDs or imply arrival.
600-callback confirmation deadline, periodic aggregate discovery/outcome metrics
and idle-tracking scheduling are implemented. All13 native invocations and nine
configuration cases pass; peak62/64 queries. Prepared build-v9a is NOT loaded.
Next one integrated live test after a fresh process and paused baseline; broader
route coverage remains separate. [Full evidence](../../docs/passenger-capacity.md).

Version0.9.1 implements station-focused discovery and direct scan-page learning.
The stopped0.9.0 trial completed three original journeys, but station-mismatch
counters showed inefficient search. All13 new native invocations pass; the
80-position fixture verifies targeted selection and catches cache-churn omissions.
Peak63/64 queries. New build-v9b is not loaded; restart is needed before its
integrated trial. See [discovery implementation](../../docs/station-discovery.md).

## Proposed extension — native alternative-platform walking

User proposes using the game's alternative-platform behavior as the foundation
for switching to another line at another platform, including checking actual
walking time versus direct boarding. This is a research/design follow-up; do not
edit platforms/lines or change guards during the active0.9.1 capture.

Current policy explicitly rejects alternative endpoint terminals and requires
matching boarding/alighting terminals plus direct waiting-edge membership.
Shipped declarations expose alternativeTerminals; native research found a
vehicle-approach walking path, NoteWalkPersonsArrived, and a direct onboard path.
Their existence does not establish alternate-platform rerouting semantics,
walking-time accounting, or teleportation. Earlier debugger traces are unsuitable
for timing measurements.

Next isolate a same-line native alternative-platform case on a disposable save.
Record configured/locked actual platform, passenger edge/path and walking states,
assignment/reservation, onboard transition and simulation time. Compare nearby,
distant and inaccessible platforms, plus departure before the passenger arrives.
Separate rendered animation from simulation timing and passenger travel-cost
accounting. Only then expose a verified native reroute operation through the
shared bridge and extend policy to cross-line platform changes. Use estimated
walking time and a boarding margin, validate actual arrival and capacity, and
preserve a valid journey if the train departs or the platform changes again.
Do not equate same station group with immediate platform access or merely remove
the existing mismatch guard. Whole-trip time estimation remains a separate gap.
See docs/native-boarding-investigation.md and src/interlining/native_candidate.c.

Research completed2026-10-09: [alternative-platform behavior](../../docs/alternative-platform-behavior.md)
records official vehicle-selection rules and verified native control flow.
The vehicle-entry walk searches within5 world units; an empty result falls
through to direct boarding. Walking arrival has its own onboard callback.
Upstream platform relocation, full transfer costs and ordinary missed-departure
handling remain unknown. Do not treat this short approach helper as a station
pedestrian router or remove platform guards on its account. No live alternative
case was introduced into the ongoing0.9.1 capture.

Current0.9.1 checkpoint6600:seven boardings,six original arrivals,one active sample,
zero logged errors. Live switching beyond five is demonstrated; unsampled
boardings and broader route cases remain pending. Capture is not yet complete.

Superseding milestone: user authorized native behavior for cross-platform support.
Version0.10.0 implements same-station boarding and different fixed alighting
platforms, ABI8 copied configured boarding options, actual-terminal checks and
deferred snapshot comparison. All13 native invocations pass; worst63/64 queries.
Prepared build-v10a/package; not loaded or live validated. No hard-coded save or
line IDs. User's S2 platform2/3 setup is a test fixture. Alternative future alighting
terminals remain excluded. [Implementation and trial](../../docs/cross-platform-trial.md).

The user cancelled0.9.1 after eight both-world boardings and eight original arrivals,
zero errors. Native stop returned0; watcher ended. Stopped partial audit passes;
unsampled and multi-stop/express live coverage remain pending. Next integrated
cross-platform run needs a fresh game process, not another read-only replay.

## 2026-10-11 — Return-terminus fix ready

Recovered0.10.0 partial capture:12 boardings,5 across platforms,4 sampled original
arrivals,4 unresolved samples,4 unsampled boardings,zero errors. No automatic
summary or explanation of capture ending. No game process remains.

Version0.10.1 fixes bounded last-to-first stop-list traversal on either line.
All13 native invocations pass,63/64 worst query budget; report/binary hashes
verified on recovery. Build-v10b/package prepared, not loaded. Next integrated
trial targets Wimborne→Faringdon; no extra diagnostics-only replay needed.
See [return-terminus fix](../../docs/return-terminus-fix.md).
