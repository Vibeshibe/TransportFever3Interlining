# Confirmed home invalidation — 0.7.5

## Why the removal deadline was insufficient

The0.7.4 live trial correctly paused new switching on home loss, then stopped at
its32-callback deadline. Passenger120770 remained present after both worlds
cleared home87062 to-1; target72649 and its original assigned slot were unchanged.
The engine does not necessarily remove such a passenger immediately.

| Observation | Callback / time |
| --- | --- |
| 120842 switches S3→S1/97745; both-world boarding | 418 / 425 |
| 122261 switches S3→S1/97745; both-world boarding | 427 / 429 |
| 120770 switches S1→S3/97914; both-world boarding | 1475 / 1477 |
| 120770 home cleared, world1 / world2 | 1981 / 1982 |
| Removal wait expires; no removal observed | 2013 |

The run ended with `home_clear_timeout`, staged3, switching_blocked1 and zero
query errors. All three original arrival masks remain0. No fourth/fifth switch
or lifecycle resolution occurred. The log-invariant audit passed; continuation
is **not** validated by this run.

Watcher48734 exited0 at17:50:14.650952Z. At17:51:19.463650Z/17:51:19.464765Z,
external repeated copied reads still found120770 with SimPersonAtVehicle,
destinations `[-1,-1,72649]` and target72649 in both worlds. At17:51:28.596278Z /
17:51:28.800613Z it had no SimPerson, only EcsOctreeNode. The follow-up completed
after9.34sec:165 accepted world samples,12 rejected inconsistent reads. It makes
no engine calls or writes; these are observational snapshots, not atomic reads.
No intermediate alighting or exact native removal call was captured.

## Revised behavior

Version0.7.5 closes tracking as **home_invalidated** once both worlds confirm
home=-1 after successful switching and expected both-world boarding. The original
target and assigned destination slots must still match, target must differ from
home, and there must be no prior arrival or destination-invalidation evidence.
No removal may have been observed when classifying home invalidation.

This outcome records loss of the home assignment, not passenger removal, original
arrival or a natural cause. The passenger may still be onboard; its journey is
no longer followed. Its ID remains excluded and its attempt remains consumed.
Once every pending case resolves, other eligible passengers may be served, unless
a separate permanent query-error block exists. The reserved final attempt can
then be used. Mixed outcomes remain distinct from all destinations reached.

One-world home loss still pauses proposals and retains the same32-callback
deadline. A restored/different home, pre-boarding loss, changed target/slots,
mixed arrival/invalidation or reappearance after removal still stops. Mixed
home-loss/removal evidence does not become home_invalidated; it may resolve only
as both-world removal before the unchanged deadline. No retry, timeout increase,
attempt refund, ABI change or native mutation change was introduced.

## Validation and next trial

Normal/reserved consumer fixtures pass, including persistent onboard home loss,
actual later different-person requests, reserved fifth release, no ID reuse or
cap refund, mixed outcomes, exact one-world timeout, late confirmation, conflicting
worlds, home-bound targets and permanent error blocks. Host batch/failure and
both mutable/read-only DLL contract suites also pass; peak allowance40/64 queries.
Build warnings are errors.

The completed live trial below validates continuation after confirmed removal.
It did not capture home loss, so `home_invalidated` remains fixture-tested only.

## Completed 0.7.5 live trial

Loaded the verified build-v7f DLL after a fresh restart into the unchanged
`TPF3Interlining_before_multi_passenger_01` baseline; loader returned 0. User
confirmed normal-speed unpause. Limits of 5 passengers and 18,000 callbacks, reserved final attempt and the 32-callback
home grace were unchanged. No new code or DLL was loaded during the capture.

| Passenger | Switch / vehicle | Stage | Both-world boarding | Final outcome |
| --- | --- | --- | --- | --- |
| 120842 | S3→S1 /97745 | 512 | 514 | Removed at 7189/7190; no arrival |
| 122261 | S3→S1 /97745 | 518 | 520 | Original destination 72934 at 2160/2161 |
| 122790 | S1→S3 /97914 | 2011 | 2013 | Original destination 86009 at 5782/5783 |
| 127558 | S3→S1 /98398 | 2986 | 2999 | Original destination 120856 at 5238/5239 |
| 86553 | S1→S3 /97914 | 9214 | 9247 | Original destination 86548 at 13694/13695 |

All five replaced the existing stops 1→2 leg and alighted from the substitute at
planned stop 2. Both worlds reported 120842 removed; its tracking closed at 7190
without an arrival claim, and discovery resumed with one attempt remaining.
The later 86553 switch and both-world boarding prove actual continuation after
confirmed removal. Its onward S2/98389 journey and connecting service 39045/79465
then completed at the original destination. No full-trip timing improvement or
specific cause of 120842's removal is established.

At 13695 the pilot ended automatically with `all_outcomes_observed`, staged=5,
query_errors=0 and switching_blocked=0. Final masks confirm **four original arrivals
and one removal**, not five successful journeys. The wrapper explicitly forwards
only and permits no further changes. Watcher 73040 exited 0 at
2026-10-08T18:53:18.205804Z after 2,767.65 seconds. All observers/readers have ended;
user was told the game may be paused or closed. No checkpoint or restart is needed
to complete this trial. The DLL remains mapped until game exit.

The final log audit passes: five unique engine attempts with preserved waiting
age/destination/place and closed engine scopes; every expected boarding confirmed;
no new mutation during unresolved lifecycle evidence; no later journey reads for
the closed ID; fifth attempt follows lifecycle resolution. Audit flags are
`continuation_proven=true`, `home_invalidation_continuation_proven=false` and
`home_clear_continuation_proven=false`. No home_pending/home_invalidated event
occurred, so the home-specific 32-callback path remains automated-test coverage.

Historical process: Linux PID 102810/start token 2320640, Windows PID 304, thread 592; worlds
`0x2e7c3ca0`/`0x2e7c4960`. Never reuse after restart/reload. Ignored metadata and
complete evidence are `build-v7f/live-run.json`, `live-audit.json` and
`tpf-bridge-304-23489567.log`. Baseline remains 80,431,012 bytes, SHA256
`ce2e02d935da822e63fbd8a12fe24234cfef6a5698d841e4f978939ca3c80f84`.
No save files were changed by our tools.

## Next implementation

Proceed with bounded route-eligibility expansion beyond the save-specific S1↔S3
adjacent leg. Preserve each passenger's planned alighting stop; do not shorten
through legs to manufacture transfers. Retain platform-access, capacity, current
state/revalidation, attempt and lifecycle guards. Home-specific live coverage
remains an explicit gap to exercise when a matching case occurs, rather than a
reason to replay this completed removal test. Broader route support needs its own
implementation and focused tests; no stable release, autoload installation,
platform switching or preference/time optimization is claimed.

## Artifact hashes

Under ignored `analysis/native-bridge/build-v7e/`:

- Log22022778: `c7e4e76ce32fa36e50b5ce7aee02c127107594356b6a38d39e6a574c6509b498`.
- `home-timeout-followup.jsonl`: `ba502fa48d285521d36c6ef203c181febce45f5f374c31c6536a984724fca86e`.
- `live-audit.json`: `2d635cb9c6a9c281d1fc61e1896b0cb9ba0231e46ee45d3bd296b7b07ea14a3e`.

Under `build-v7f/`:

- Mutable DLL: `34ad73a8d3aaad513899327ebeaafe22167a47f3ef5d0c99668044f552685b60`.
- Observer DLL: `c9e69dedeb4e870e5daaee60a003e1b9fa1dae7c40dbd2aef8fd96cc4745ac78`.
- `home-invalidation-tests.json`: `b8c4506ae3f3c6f25493e44c6c272890cf46ab124ff714d32e2cdc3ba4a0bade`.

Live evidence under `build-v7f/`:

- Log23489567: `50f6f792028fe0ef4b4e67d033c9046067a0bff9a50299ad8e4beaf315915c4d`.
- `live-audit.json`: `1957a0824c518bd4d192aeaed5a4d387005cdb0cded81ca0166dfa6b2cc58c63`.
- Completed `live-run.json`: `3fe02395cbd7b1cc8616c6e88709437672e601df2c8ddb860a69827f3ee16e9a`.
