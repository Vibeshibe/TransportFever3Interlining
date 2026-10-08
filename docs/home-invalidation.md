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

Prepared ignored build directory `analysis/native-bridge/build-v7f/`, cap5/18000,
reserve enabled. Loaded after the user restarted into the verified before_multi_passenger baseline;
loader returned0. Fresh Linux102810/start2320640, Windows304; log
`tpf-bridge-304-23489567.log`. User confirmed normal-speed unpause; watcher73040
is active. **Live outcomes remain pending.** No save was modified by our tools.
Require home_invalidated confirmation, resumed discovery and an actual later
different-person switch/boarding, with separate final journey outcomes.

## Artifact hashes

Under ignored `analysis/native-bridge/build-v7e/`:

- Log22022778: `c7e4e76ce32fa36e50b5ce7aee02c127107594356b6a38d39e6a574c6509b498`.
- `home-timeout-followup.jsonl`: `ba502fa48d285521d36c6ef203c181febce45f5f374c31c6536a984724fca86e`.
- `live-audit.json`: `2d635cb9c6a9c281d1fc61e1896b0cb9ba0231e46ee45d3bd296b7b07ea14a3e`.

Under `build-v7f/`:

- Mutable DLL: `34ad73a8d3aaad513899327ebeaafe22167a47f3ef5d0c99668044f552685b60`.
- Observer DLL: `c9e69dedeb4e870e5daaee60a003e1b9fa1dae7c40dbd2aef8fd96cc4745ac78`.
- `home-invalidation-tests.json`: `b8c4506ae3f3c6f25493e44c6c272890cf46ab124ff714d32e2cdc3ba4a0bade`.
