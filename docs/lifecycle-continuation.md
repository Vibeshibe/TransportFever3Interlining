# Continue after confirmed lifecycle outcomes — 0.7.3

## Implementation

Version 0.7.2 ended the entire pilot after a confirmed removal or cleared
destination. Version 0.7.3 closes that passenger's tracking and allows other
eligible passengers to be served within the same bounded run. ABI 7, native
mutation calls and the S1↔S3 adjacent-leg eligibility checks are unchanged.

Continuation requires a successful change result, expected boarding in both
worlds, and matching removal or destination-invalidation evidence in both worlds.
The first lifecycle observation pauses discovery. Every pending passenger must
be resolved before discovery resumes. A query failure uses a separate permanent
block; lifecycle confirmation cannot release it. The existing error/callback
bounds remain. Disagreement, reappearance before confirmation, reassignment,
changed home, mixed arrival/lifecycle evidence, or lifecycle loss before confirmed
boarding still stops the pilot. No rollback or retry was added.

Closed passenger IDs remain in the attempted-ID ledger, consume their original
attempt, and are never queried or selected again. This avoids following a reused
ID or a later journey. Lifecycle outcomes never increment the arrival masks.
Previously completed arrivals remain complete. When all five attempts have known
outcomes, the run ends with `all_destinations_observed` only if all five arrived;
otherwise it ends with `all_outcomes_observed` and separate per-person outcomes.

This classification records observable lifecycle state, not a natural cause or
a stable entity generation. It does not establish why a passenger was removed.

## Bounded trial pacing

The previous capture exhausted all five attempts before removal, so it could not
demonstrate continuation with unused allowance. The optional, default-off
`--reserve-last-attempt-for-lifecycle` build input holds the last attempt until
one tracked lifecycle outcome is confirmed. It changes only trial pacing, not
eligibility, passenger state, the attempt cap or the mutation backend.

The next trial permits four initial attempts, then continues tracking until a
confirmed lifecycle outcome releases the fifth. If no such outcome occurs,
the callback limit ends an inconclusive continuation test; do not infer success
or expand the limits automatically. The flag requires a passenger limit of at
least two and is logged at startup. It is not an SDK/runtime override.

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v7d \
  --passenger-limit 5 --observation-limit 18000 \
  --reserve-last-attempt-for-lifecycle \
  --observe-person 122261 --observe-destination 72934
```

Load only the mutable `tpf_shared_bridge.dll` for the switching trial. The
optional observer binary is built for compatibility tests, not simultaneous use.
Do not rebuild a loaded DLL. A fresh game process is required for this new build.

## Validation and next live test

Consumer fixtures exercise actual new requests after removal and invalidation,
multiple pending passengers, unrelated query failures, attempted-ID retention,
unchanged capacity policy, first-boarding and identity guards, mixed outcomes,
callback caps and no refunded attempts. A second consumer executable,
`pilot_reserve_tests.exe`, tests the actual reserve build: no fifth request before
confirmation, a fifth afterward, and clean timeout if all initial journeys arrive
without a lifecycle event. Invalid reserve/cap combinations reject in both CLI
and CMake. All these checks passed. The maximum consumer fixture allowance remains
40/64 queries; no budget increase was needed.

For the live test:

1. Restart and load a disposable copy of `TPF3Interlining_before_multi_passenger_01`
   paused. Baseline SHA-256 is
   `ce2e02d935da822e63fbd8a12fe24234cfef6a5698d841e4f978939ca3c80f84`.
2. Verify the fresh process/save and DLL hash. Load build-v7d with its loader in
   the game's Proton prefix. Record a new log, process identity and world addresses.
3. After successful startup, unpause at normal speed without vehicle/line edits.
   The cap remains five attempts and 18,000 callbacks. User action is needed only
   after implementation/automated validation is complete.
4. Require `pilot_lifecycle_resolved` and `pilot_tracking_resumed` before a later
   `pilot_staged`/applied change for another passenger. Check both-world boarding
   and the final per-person outcomes. A resumption log without an actual later
   successful change is not full continuation validation.

Live validation is pending. No revised DLL has been loaded into the game.

## Completed automated validation

The build completed with warnings treated as errors. Normal and reserved consumer
fixtures passed, along with host batch/failure, candidate-policy and both mutable
and read-only DLL contract tests. ABI6/7 capabilities remain127/255 for mutable,
95/223 for read-only; unsupported executables still reject. The native query and
mutation implementations did not change. One fixture initially scheduled an event
on a discovery callback; its alignment was corrected before the final passing run.

Ignored artifacts under `analysis/native-bridge/build-v7d/`:

- Mutable DLL SHA-256:
  `6c29dc15804889f319f471d81ff5b04f25b15a5047b51bc915d0b6562609b39c`.
- Observer DLL SHA-256:
  `54b85a9a7a9b98ce1a45663bc3602190283ea7a301b4d49ad20717e8c7d8e475`.
- `continuation-final-tests.json` SHA-256:
  `a1136aa0a3919c86445e826a20b562775b0f9e08acfe999dc82edd934c7ba984`.
- `continuation-hashes.json` records final artifact hashes. Trial CMake cache
  confirms cap5/18,000 and reserve enabled. Invalid cap1/reserve combinations
  reject before building through both the helper and direct CMake configuration.

The preserved pre-trial save hash was rechecked unchanged before requesting the
live load. No game process was modified during implementation or automated tests.

## Live startup — 2026-10-08

Implementation `6cab818` is pushed. Fresh Linux79516/start1873255 / Windows308;
user corrected the initial save selection before any bridge injection. Stdout
confirms loading `TPF3Interlining_before_multi_passenger_01` at16:50:20Z and ready
at16:50:53Z. Process identity, absent previous bridge and DLL hash were verified.
Loader returned0. Log `build-v7d/tpf-bridge-308-18964006.log` confirms0.7.3,
cap5/18,000, continuation policy and reserve1. User has been asked to unpause at
normal speed; live outcomes and world addresses are pending. Do not rebuild the
loaded DLL. Process metadata is preserved in adjacent `live-run.json`.
