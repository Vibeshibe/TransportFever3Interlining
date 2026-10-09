# Bounded capacity and sampled journeys — 0.9.0

The 0.9.0 live trial was stopped after three confirmed switches, boardings and
original destination arrivals. Sample-slot reuse was observed; **more than five
switches and unsampled-boarding live coverage remain pending**. Version 0.9.1
adds [station-focused discovery](station-discovery.md); a new live trial is pending.
The earlier [design](passenger-capacity-plan.md) is retained for context.

## Behavior and limits

The bridge supports **1–32 distinct attempts per DLL lifetime**, default one.
Every replacement still requires fresh eligibility and deferred route/state
revalidation, followed by confirmation of the expected vehicle, line and stops
in both engine worlds. Only one boarding can be unconfirmed at a time.

The consumer separates a compact attempt ledger from reusable journey trackers.
The host independently retains attempted IDs across clients and worlds. Releasing
a tracker never refunds an attempt or permits another change to that person, even
if the ID later represents a different entity. The native mutation operation,
copied-query ABI7, endpoint/platform/capacity policy and failure latch are unchanged.

With the prepared32/4 configuration, at most four confirmed journeys receive
detailed tracking, plus one pending boarding: **five active trackers maximum**.
After boarding, a free sample slot retains that journey; otherwise its tracker
closes as `boarding_confirmed`, with final arrival unobserved. Completed sample
slots are reusable. Sampling is deterministic, first available slot at boarding,
and is not statistically representative of all passengers or journey lengths.

One-to-five-attempt builds default to tracking every journey for regression work.
`--journey-sample-limit` selects fewer concurrent samples; runs above five attempts
permit at most four. The lifecycle-reserve option requires full tracking.

## Scheduling and failure handling

Pending boarding is checked every callback and must confirm within600 callbacks
of staging. An unresolved engine result, missing replica or late boarding stops
new changes at that deadline, without retry, refund or rollback. These are dispatch
callbacks, not seconds. At the overall capture boundary, `callback_limit` takes
precedence over the boarding timeout; unconfirmed state remains unresolved.

Detailed tracking and discovery use separate callback phases, covering both
replica worlds. Pending lifecycle evidence freezes new proposals; the existing
32-callback one-world home-loss deadline and identity/error guards remain.
Once no trackers are active, all available callbacks can perform discovery.
Post-boarding lifecycle changes on unsampled passengers are not observed.

Version 0.9.0 retained32 people,16 lines and16 vehicles. Version 0.9.1 adds
a bounded station index while retaining four vehicle/passenger screens and
at most one full candidate validation per discovery callback.
No platform restriction or route guard was relaxed to increase matches. The cap
does not guarantee32 opportunities or establish simulation-time performance.

## Diagnostics

Every600 callbacks and at normal consumer shutdown:

- `pilot_totals` reports staged/applied/boarded counts, cumulative sample count,
  sampled arrivals/lifecycle outcomes, unsampled confirmed boardings, unresolved
  tracked outcomes, active/peak trackers and pending boarding.
- `pilot_discovery` reports scan, waiting, line, vehicle and screening work,
  stale/selected IDs, same-line/assigned exclusions, station mismatches and full
  validation calls. Existing aggregate candidate-reason counters remain.

Counters describe repeated work, not unique rejected passengers. Unsampled
boardings never add arrivals. The sample count is cumulative and can exceed four
as slots are reused; active samples cannot. `sampled_outcomes_observed` ends a
fully consumed sampled trial once retained journeys finish. It does not mean
every passenger reached a destination. A callback-limited run can be incomplete.

## Build and offline evidence

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v9a \
  --passenger-limit 32 --journey-sample-limit 4 --observation-limit 18000 \
  --observe-person 75561 --observe-destination 79212
```

The additional observer is for contract testing only, not concurrent loading.
Warnings-as-errors build and all13 native invocations pass: candidate, normal,
reserved and sampled consumers; shared bridge; copied queries/mutation; five-person
host normal/failure;32-person host normal/failure/history; both DLL contracts.
The actual consumer fixture performs32 unique switches with four samples and28
released boarding trackers. It verifies slot reuse, retained-ID exclusion, no
journey/lifecycle reads after release, deadlines, final counts and query bounds.
Peak conservative query allowance remains **62/64**, including deferred validation.

Nine option combinations were checked through C guards and CMake; all six invalid
CLI combinations reject before building. Evidence under ignored `build-v9a/`:

- Mutable DLL: `226347e0a0befe95837f3c4dfcf0831e87fc2c5ecc263d96fef1760320dbf14c`.
- Observer DLL: `a8293373faf78eebc4771dbd8b2241af4b82316255612500e14f446eb8694ece`.
- `capacity-tests.json`: `928472786c99aecdc669826553a543f43055b0d620af9cce86614658622544b9`.
- `configuration-tests.json`: `d08f5a80efe248221c8f988226fa1321995446a8578e336af5469dc41e7a94b9`.

## Next integrated live test

Use a fresh game process and disposable `TPF3Interlining_before_multi_passenger_01`
baseline, initially paused. Verify process/start identity, actual loaded save,
DLL hash and absence of an earlier bridge before loading. The stopped0.9.0 remains
mapped until its process exits. No game or save was modified during implementation.

Run32 attempts/18,000 callbacks/four samples with no line or vehicle edits.
Check both-world boarding, peak trackers≤5, sample release/reuse, truthful final
counts, zero query errors and discovery counters. More than five switches and an
unsampled boarding require sufficient eligible demand; report those coverage
gaps if they do not occur. Do not keep extending the capture merely to fill a cap.

Multi-stop/express replacement, special waiting exclusions, whole-trip time/fare
comparison, platform walking and varied passenger preferences retain their
separate evidence gaps. This is a bounded development loader, not an unrestricted
or automatically installed release mod.

## Historical live startup — 2026-10-09

Fresh Linux68800/start1296244, Windows308; pinned executable and DLL verified,
no prior bridge mappings. Actual stdout load is the unchanged original baseline,
load11:15:52Z and ready11:46:44Z. Loader returned0 and the startup log confirms
32/18000/four samples,600 boarding grace and reserve OFF. Log under build-v9a:
`tpf-bridge-308-14937416.log`. Watcher28281 active; normal-speed unpause requested.
At checkpoint6300, two adjacent replacements boarded and one original journey
completed; the second used S1 stops4→5 to S3 stops5→6 and is on its native onward
connection. Sample-slot reuse is observed (two cumulative samples, peak1 active).
Zero query errors; no unsampled boarding or >5 switches yet. The DLL remains mapped
until game exit; do not load a second bridge into this process.

## Stopped outcome — 2026-10-09

`tfx_stop` returned success at12:21:00Z; the watcher ended. Passengers122261,
127526 and120415 each boarded and reached their original destinations in both
worlds. Latest totals at9000 show three samples, peak one tracker, active zero,
and no query errors. Last candidate log9419; manual stop emits no final summary,
so an exact stop callback and final aggregate counts are unavailable. All three
replacement legs were adjacent; >5 switches and unsampled boarding remain untested
live. See [station discovery](station-discovery.md) for the next implementation.
