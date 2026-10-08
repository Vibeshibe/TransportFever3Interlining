# Repeat journey and save/reload trial

## Purpose and limits

Repeat the successful same-platform S1↔S3 switch using revision 0.6.2's rolling
passenger discovery. Then save the altered simulation and reload it in a fresh
game process. Observe the same passenger without enabling further changes.
Keep the original baseline and S2 platform layout unchanged.

A successful reload after arrival tests loading the altered simulation. A save
while the switched passenger is onboard is stronger evidence: the saved route,
vehicle and destination should survive and the journey should finish unaided.
Record which state was actually saved; do not substitute a post-arrival reload
for evidence of preserving an in-progress journey.

## Procedure

1. Restart the game; load a disposable baseline and leave it paused. Load the
   preserved 0.6.2 pilot from `analysis/native-bridge/build-v6c/`. Its SHA-256 is
   `bce9aedbc8b3685dc7497ec5ab9ab5b89642a994c52c1bc8bb5afcf3ca863fe1`.
2. Unpause at normal speed. Monitor the log for one applied replacement and
   onboard state in both worlds. Record person, original/new line, vehicle,
   boarding/alighting indices and destination.
3. Pause when notified and save under a new name, such as
   `TPF3Interlining_after_switch_01`. Record the last observed journey state and
   retain a hash/copy of the save under ignored `analysis/`. Never overwrite the
   original baseline. If arrival occurs before pausing, label the test accordingly.
4. Exit the game completely. Reload that new save and leave it paused. Build the
   observer for the recorded person and destination in a fresh directory, then
   load **only** `tpf_journey_observer.dll` using the normal development loader.
5. Unpause and compare its first state with the saved checkpoint. Follow boarding
   or onward movement to arrival where applicable. Record a mismatch or missing
   passenger as unresolved evidence, never an automatic success. Entity IDs alone
   do not establish identity across unrelated saves or entity reuse.

## Observer build

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/reload-TRIAL \
  --observe-person PERSON_ID --observe-destination DESTINATION_ID
```

The paired IDs are nonnegative int32 values stored in the build's CMake cache.
Without both options, no observer target is configured. The observer reuses the
verified terminal boundary, fresh world discovery and copied journey queries.
It links neither the mutation backend nor candidate policy, dispatches without a
writer, and omits the waiting-change capability. The ABI retains its layout;
mutation requests in read-only callbacks return `TFX_QUERY_UNAVAILABLE`.

It logs changed state, ends at 3,600 callbacks, eight query failures, or the
expected building observed in worlds 1 and 2. A building observation is reported
separately from proof of boarding after reload. No engine world pointer, callback
token, request ticket or cached component survives the restart. The observer
installs the same single-owner hook and remains pinned until process exit.
Use a new process between pilot and observer; do not load both together.

## Prepared validation

The first 0.6.3 observer build uses the previous trial's person 72845 and destination
72405 solely as build inputs. These must be replaced with the next trial's actual
IDs. It has not been loaded into the game. Offline tests cover two-world arrival,
wrong destination, mixed states, missing passengers, invalid worlds, callback
limit, and rejection of mutation requests during read-only host dispatch.

All four suites passed under the isolated Wine prefix: `journey_probe_tests`,
`bridge_tests`, `query_tests`, and `candidate_tests`. The observer DLL itself
reported capabilities 95 and rejected the unsupported test executable. Missing,
negative and overflowing observer arguments were rejected before building.

Prepared observer SHA-256:
`3ee8d63daf407fa8b1d3d7be7b3074e940cc8dc4e828db1a6328fded9a8b2ece`.
Offline validation log: `analysis/native-bridge/build-v6d/offline-validation.log`,
SHA-256 `8eceb9ac837d16b552d44e9492ccf552d9940f9a2d61431994944f61fca026c6`.

## Completed repeat capture

The user confirmed the disposable baseline loaded and paused. The preserved
0.6.2 pilot loaded into Windows PID 312 with `tfx_start=0`; the log confirms the
verified slot installation. Capture log:
`analysis/native-bridge/build-v6c/tpf-bridge-312-5551718.log`.
The user was asked to unpause at normal speed and stay ready for an onboard
checkpoint. No replacement or reload result is established by initialization.
The user confirmed normal-speed unpause; callbacks arrived in both worlds on
thread 620. Early candidates were rejected for boarding mismatch. No mutation
had yet been logged at that intermediate checkpoint.

At callback 1958, passenger 108583 passed the full validator for S3→S1,
vehicle 98610, with 40 places remaining. Existing S3 stops 5→6 matched proposed
S1 stops 4→5, boarding station 97294 terminal 1 and alighting station 97948
terminal 0, destination 75521. The pilot selected only original stops 1→2 and
did not stage this return-leg candidate. A read-only inspection of the bridge's
own counters then found 1,973 accepted callbacks and zero rejected boundaries.
No debugger or memory write was used for that counter check.

## Longer observation window

The user clarified that this is the original baseline with recently established
routes and sparse demand, and requested more time. Eligibility is unchanged.
Prepared revision 0.6.4 at `analysis/native-bridge/build-v6e/` with
`--observation-limit 18000`. The one-attempt cap and original S1/S3 stops 1→2
selection remain unchanged. Every 300 callbacks now logs progress. The read-only
reload observer keeps its default cap of 3,600.

This build is separate from the now-disabled 0.6.2 DLL. Save the progressed
simulation under a new name before restarting for the longer capture. Such a save develops demand; it is not a switched-passenger checkpoint.
No live cap, instruction, engine field or guard was patched to extend the run.

## Repeat result and reported Wimborne example

The 0.6.2 run auto-disabled at 3,600 callbacks: 26 remembered people, 12 vehicles,
86 ready-vehicle samples, 28 full evaluations, one accepted return-leg candidate,
seven unsupported-leg rejections and twenty boarding mismatches. There were zero
query errors, rejected boundaries, staged requests or engine attempts. Discovery
never filled its 32-person allowance, so this run did not exercise rolling eviction.
No switched-passenger save checkpoint was available. User was asked to save the
progressed simulation and restart into it for the longer trial; confirmation is
pending.

After capture ended, user reported an S1 arrival at Wimborne with a person waiting
for S3. The new external `boarding_snapshot.exe` used read-only process access,
existing copied queries and fresh terminal-system lookup in the supplied worlds.
World 1 completed a 112-page scan with 13 baseline-line waiting passengers and
matching repeated waiting/vehicle reads; its modification scope was idle before
and after. World 2 did not satisfy idle/system preconditions and was skipped;
the helper exited 3 to report partial evidence. These are observations, not an
atomic snapshot or callback-authorized candidate validation.

In world 1, person **75515** waited on S3 (`97743`) stops **1→4**, destination
**86142**, with direct platform membership **97948:1**. S1 vehicle **98594** was
at stop 1 on the same platform, locked, state 2/load state 4, with 40 free places,
zero onboard and zero reservations. S3 stop 4 is station **97699**, which is absent
from S1's enumerated route. S1 cannot preserve this passenger's planned alighting
stop; using it would need a transfer and onward replanning. The pilot's
unsupported-leg rejection is therefore appropriate here. User acknowledged this
was not the intended shared-leg example. No engine changes were made.

## Additional artifacts and validation

| Artifact under `analysis/native-bridge/` | SHA-256 |
| --- | --- |
| `build-v6c/tpf-bridge-312-5551718.log` | `366113708b5f98d48dc24a4ca80f641989969403fedfe3e06495936063a412ad` |
| `build-v6e/tpf_shared_bridge.dll` | `5a649b0aa32b63f35f5d88646136452b6b837fb2d8cfb1cffc8bba414622c657` |
| `build-v6e/offline-validation.log` | `c568189faf8363dcaa029cc11d286abb5d6c3c6494b763c4ed93396d9194fb78` |
| `build-v6e/boarding_snapshot-captured.exe` | `e2173392a7f9ae7d2581f656dd60654d8bcbbb49e4dc0ef3e7de8bbdeb5774ac` |
| `build-v6e/wimborne-opportunity-01.log` | `c8a3d64065a6ed8910466218564ee58484edbd52508b5542ab0a466fe73d16bd` |

The longer pilot compiled with warnings as errors. Candidate and journey-observer
suites passed; invalid observation limits 0, -1 and 36,001 were rejected and build
flags confirm the pilot cap of 18,000. Eligibility, mutation backend and host
attempt limit are unchanged from the passing 0.6.3 suites. No new live replacement
or save/reload result is claimed. The captured external helper is preserved before
adding its CMake target and tightening incomplete-scan/platform error reporting.

## Longer trial loaded from preserved warm-up save

The user confirmed restart into the progressed save, paused. Linux PID 42420 /
Windows PID 312 had no existing bridge mapping. The recorded 0.6.4 DLL loaded
successfully (`tfx_start=0`) and logged its 18,000-callback limit. New log:
`analysis/native-bridge/build-v6e/tpf-bridge-312-6954251.log`.
User confirmed normal-speed unpause; both worlds began observation without
rejected boundaries (thread 600; world 1 `0x2d2f8580`, world 2 `0x2d2f78c0`).
No replacement or save/reload result follows merely from successful startup.

The warm-up save was copied into ignored
`analysis/baselines/04569232cb31/TPF3Interlining_warmup_01.sav`, 77,688,477 bytes,
SHA-256 `04569232cb31adfdec45d83745b21aeb9179944d745c436be6cc6897414644d5`.
The copy hash matched and source size/mtime were unchanged across copying.
Its manifest records source modification time `2026-10-08T10:45:30.502598+00:00`.
This preserves demand development; the switched-passenger checkpoint is pending.
