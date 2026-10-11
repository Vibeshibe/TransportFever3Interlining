# Continuous operation — 0.13.0 release candidate

Status: implemented on `dev`; offline validation passed. Live validation and
stable promotion remain pending. This is still explicitly loaded with the native
loader, not an automatically installed mod. Only the pinned executable is supported
(SHA-256 `5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efa93fef34146b1`).

## Behavior

The two recorded Wimborne→Faringdon S1 departures served S2 waiters while S3
waiters remained. Neither event exhausted the attempt cap. Individual remaining
S3 passengers were not captured, so their exact eligibility is unproven; see
[the original evidence](boarding-window-selection.md).

Selection now ranks the planned alighting **station group across source lines**,
trying a different compatible source line before returning to the last served
line. General exploration still runs on one of four preferred-selection turns.
Hints never authorize mutation: current waiting state, route, terminals, capacity
and destination are revalidated before the engine changes the waiting component.
This adds no arrival prediction, cheapest/fastest personality or guaranteed queue
clearance. Existing planned legs and original destinations remain the policy.

`--continuous` removes the lifetime 32-attempt and observation-callback cutoffs.
Thirty-two recent outcome records are recycled into aggregate counters; at most
four complete journeys and one unconfirmed boarding are tracked simultaneously.
Every changed passenger still requires boarding confirmation in both worlds.
Later journeys by the same person become eligible only after that confirmation
and a different native waiting timestamp. The timestamp is compared as opaque
bits; the current and immediately preceding episode cannot be replayed.

The host and consumer each use a sparse radix map indexed by entity ID. Lookup
cost does not increase with total switches; allocated memory follows entity ID
pages touched, not the number of journeys. Allocation precedes mutation. An
uncertain write still blocks subsequent changes. Stale/policy-rejected requests
with no engine attempt can be discarded and rediscovered.

## Network discovery and retained bounds

ABI9 adds copied, paged `scan_lines` queries; ABI1–8 layouts remain supported.
A separate phase visits line entities even when they have no waiting passengers.
New/deleted/recreated lines are discovered incrementally. Stop counts and proposed
routes are reread, and route edits invalidate stale proposals.

Continuous does **not** mean unbounded work within an update:

- At most 64 queries, 8,192 copied reads and 1 MiB copied bytes per dispatch.
- Each scan page inspects at most 1,024 entity masks and returns 16 IDs.
- Working hints hold 64 lines, 128 vehicles and 64 station groups with up to
  128 people each. Cursors and eviction continue discovering other entities;
  these are not a permanent whitelist. Very large-network coverage is unproven.
- Existing maximum four-stop-interval route policy, terminal rules, boarding
  deadlines and lifecycle checks remain. Alternative alighting terminals remain
  excluded. A busy station can still have ineligible or undiscovered passengers.
- One process/save session per load. Restart before changing DLL or loading a
  different save; automatic save-session reset/reloading is not implemented.

## Offline evidence

All 17 build-v13a suite invocations passed, with verified executable/DLL hashes.
The suite exercises the real consumer and policy with mocked game
queries, plus separate native query/host tests and actual DLL API checks:

- 5,185 staged requests over 66,500 callbacks; repeated later journeys, record
  recycling, duplicate suppression, newly created demand-free service, deletion/
  recreation, and rejection both before staging and at deferred validation.
- Host stress: 4,096 episodes across recycled IDs; both-world/client protection,
  wrong-vehicle observations, stale replay and uncertain-write failure latch.
- Same mixed-direction fixture: all 20 compatible waiters served in 120 callbacks
  across 16 phases, both source lines served in every phase. The recorded 0.12.0
  comparison served 12–13. Two opposing trains each served at least ten waiters
  in all eight phases. These are synthetic timing windows, not live guarantees.
- Peak fixture query counts remain within 64; existing worst-case policy tests
  reach 64/64. Line-page tests cover sparse/dense pagination and failure output.

A 2,000-sample native-memory benchmark uses real `ReadProcessMemory` calls under
Wine against synthetic layouts. Final measurements: sparse 1,024-entity page
mean 3.769 µs / p95 3.800 µs / maximum 1,060.600 µs (12 reads); 64 waiting
queries mean 209.622 µs / p95 396.700 µs / maximum 979.900 µs (1,353 reads). This measures query cost, not FPS or mutation cost.
The mocked continuous consumer averaged 3.222 µs per callback, which excludes
native copied reads. Exact runs and fingerprints are in the ignored build report.

Production telemetry reports cumulative mean, histogram p95 upper bound, maximum
callback time, and peak query/read/byte consumption every 1,200 observations.
Timing covers reader setup, consumer/host dispatch and changes; it excludes the
original game update, entry checks and the telemetry write itself. Durations round
up to whole microseconds. Detailed passenger logs stop after 2,000 lines; aggregate
progress/errors continue, without a per-change disk flush. Log files are not rotated.

## Reproduce and release gate

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v13a --continuous \
  --observe-person 106847 --observe-destination 72810
rtk proxy python3 tools/test_native_bridge.py \
  --build-dir analysis/native-bridge/build-v13a
```

Observer IDs are separate read-only test-build inputs; the continuous consumer
contains no baseline-specific passenger, line, station or vehicle IDs.

Before promoting `main`, use a freshly restarted disposable baseline to confirm
more than 32 boardings, S1/S3 queue service at repeated return departures, sampled
original destinations, later journeys by a previously switched person and actual
callback costs. Then exercise line/stop edits and newly created service, plus
save/restart compatibility. Compare responsiveness with the unmodified baseline;
record workload and outliers rather than treating synthetic timings as an FPS
promise. Large-network churn and automatic installation/session reset remain
separate release limitations. No stable label or main merge is justified yet.

Final mutable DLL SHA-256:
`4a39e2765bf0b8388945e71f6722e8533df03b50d27d3494cc151fb2b9fc201f`.
Final report SHA-256:
`dd1702a5a0c0aa3dbfde41a929bab33e2b5714e3d94718d2261c8e5e345da5ee`.

## 0.13.1 — rank reachable endpoints before the first switch

The 0.13.0 live run still missed Wimborne→Faringdon queues. On S2 vehicle98388,
only96845 switched from S1 (1235→1237), despite37 spare places at validation.
S1 vehicle98398 subsequently recorded no switched boardings at that stop; logged
checks1486/1490 selected outbound passengers, while ready samples reported32,
25 and20 free places. The user reported5 S3/17 S1 waiters before S2 arrived and
no S3→S1 boarding afterward. Individual remaining passenger records were not
captured. Faringdon-bound passengers on that shared leg are intended candidates;
these observations do not justify excluding their queue.

0.13.1 prepares a train's reachable station-group hints before its first successful
switch when a station's indexed queues have mixed endpoints. It reads at most the
next four stop intervals, wraps the stop list, and stops at a repeated station.
Both source lines and all hinted reachable endpoints can then be prioritized.
Unambiguous queues need no additional warm-up callback; general exploration
remains available. Hints refresh when the observed line/stop changes, station or
stop count changes, or after128 callbacks when the mixed queue is revisited.

Prefetch uses a separate callback rather than adding queries to the full validation
and deferred mutation path. Cached hints still authorize nothing: stale endpoint
or route data must pass fresh policy checks. Lifetime cutoffs remain disabled in
the continuous build; no change to native mutation, ABI9, episode guards or sampling.

New regression:80 rail waiters across two source lines, with64 opposite-direction
passengers encountered before16 compatible passengers, plus40 bus stations and81
vehicles. In16 timing phases with12-callback boarding windows, the exact0.13.0
consumer served0 passengers in every window; the revised consumer served1–3 in
all16 windows. Both serve20/20 in the existing120-callback fixture. The new version
also serves at least12 per opposing train across8phases, compared with10 before.
This reproduces a search-order failure; it does not claim full live queue clearance.
Comparison artifacts are under ignored`analysis/native-bridge/build-v13b`.

Prepared build command uses the same continuous/observer options above with a new
`--build-dir analysis/native-bridge/build-v13b`. Never rebuild loaded build-v13a.
Existing lifecycle/pilot tests retain64/64 worst-case query usage. All17 final suite invocations passed; executable and DLL hashes match the
report. Live validation of0.13.1 is pending; do not promote it as stable yet. Current0.13.0 remains running independently.

Live0.13.0 processing at3,600callbacks:23 confirmed boardings,3 sampled original
arrivals,4 active samples,16 unsampled boardings; mean167.476µs, histogram p95
upper512µs,max1150µs,peak46queries/2413reads/52477bytes. No error/stop marker at
this checkpoint. Continuous mode passed its configured default3,600-callback
research limit; live passage beyond32 switches remains pending.

0.13.1 verified artifact hashes:

- `tpf_shared_bridge.dll`: `d61dd61ef9c6c506bfcfe4e4c42b54b780673c8771852eacf2a4513da36ad7a7`
- `capacity-tests.json`: `5b7e076ecc2257cc289d5f907637f882cbde63d890b25c09d4cabd430c42aa77`
- `prefetch-comparison.json`: `ea379237ce9614f1327c36112088b5c5b27cdf9362c93d17fb7613f808089602`
