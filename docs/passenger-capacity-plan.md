# Passenger capacity and sampled diagnostics

Status: historical design, now implemented offline in0.9.0. See the
[implementation, evidence and live test](passenger-capacity.md). The running game's
older DLL has not been replaced; the proposal below records the original plan.

## Separate three responsibilities

The current `native/candidate_probe.c` stores a full journey tracker for every
attempt and visits all unfinished trackers during tracking callbacks. The host
also keeps attempted person IDs in `native/host.c`. Both use
`TFX_PASSENGER_LIMIT`; merely increasing that constant couples more switches to
more per-callback queries and detailed logs.

The next bounded build should separate:

1. **Attempt ledger:** retain every attempted person ID for the session, including
   failed or retired journeys. This prevents duplicate changes across replica
   worlds and clients. Never recycle these IDs merely because diagnostics end.
2. **Boarding confirmation:** retain one pending replacement until its engine
   result and expected vehicle/line/stops are confirmed in both worlds. Every
   switch gets this check, fresh eligibility validation and deferred route/state
   revalidation. No second change while confirmation is pending.
3. **Journey diagnostics:** retain a small, explicitly selected sample after
   boarding. Unsampled passengers continue under native simulation; releasing a
   tracker means only `boarding_confirmed`, never destination arrival.

## First bounded implementation

Propose a maximum of **32 attempts**, one pending boarding and at most **four
concurrent detailed journey samples**. Keep the five-attempt full-tracking mode
available for focused route regressions. Build options must distinguish attempt
capacity from diagnostic capacity and reject invalid combinations.

Use deterministic sampling and log the selection rule. Fill free sample slots
without evicting unfinished journeys or pending lifecycle resolution. If all four
are occupied, finish the new passenger's boarding confirmation and count it as
unsampled. Retain separate attempted, applied, boarded, sampled-arrival,
sampled-lifecycle and unresolved counts. Report the sample denominator; do not
extrapolate arrivals to passengers no longer observed.

Preserve the host's once-per-person guard, one engine attempt per dispatch and
failure latch. Keep the copied-query ABI and native mutation operation unchanged.
The 64-entry host result ring is separate from the attempt ledger: stale tickets
must remain rejected, and pending results must not be overwritten.

## Work and failure bounds

Keep discovery and detailed tracking in separate callback phases. Pending
boarding and unresolved sampled lifecycle transitions take priority; sampling
must cover both engine worlds rather than repeatedly visiting only one.
At most four sampled journeys plus one pending boarding preserves a small active
tracking set as the attempt ledger grows. Measure the actual worst query cost,
including deferred validation, against the existing 64-query allowance.

Choose and fixture-test an explicit boarding-confirmation deadline. A timeout or
unexpected engine/identity state stops further changes and reports unresolved
status; it must not silently release the slot or attempt an unverified rollback.
Retain existing sampled lifecycle guards, including the 32-callback deadline for
one-world home loss. No claim is made about later lifecycle outcomes of unsampled
passengers.

Discovery also needs measurement: it currently holds 32 people, 16 lines and 16
vehicles, samples four vehicles/people and performs at most one full validation
per discovery callback. A higher attempt cap alone does not improve this coverage.
Add bounded aggregate counters for discovery work and rejection reasons before
choosing a station-indexed cache or larger discovery windows. Cache entries remain
hints; fresh native records must authorize every change.

## Verification before another live run

- Exercise more than five successful replacements while only four full trackers
  remain active; assert bounded query work and summary counts.
- Verify duplicate exclusion after tracker release, both-world confirmation,
  timeout/failure stops, sample-slot exhaustion, cap exhaustion and stale tickets.
- Preserve sentinel, route-edit, branch, platform, capacity and lifecycle fixtures.
- Build and run host, consumer, copied-query and DLL-contract tests before asking
  for one fresh game session. That run should validate the integrated capacity
  change; no separate save reload is needed for each fixture or internal step.

Multi-stop and express boarding need their own live evidence. Neither an adjacent
journey nor sampled capacity validation supplies it. Individual fare/time
preferences, platform walking, unrestricted operation and release installation
remain separate milestones.
