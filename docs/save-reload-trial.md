# Repeat journey and save/reload trial

**Result: complete for this checkpoint.** Passenger 106847's alternate S3
boarding survived an onboard save and game restart. With only read-only observers
loaded afterward, the passenger transferred to S2 and reached original destination
85882 in both engine worlds. See the final result below for hashes and limits.

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
progressed simulation and restart into it for the longer trial; confirmation was
pending at that stage.

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
This preserved demand development before the switched-passenger checkpoint below.

## Second switch and onboard checkpoint

At sequence **1119**, the longer trial staged and applied one S1→S3 change for
person **106847**, vehicle **97913**, shared stops **1→2**, destination **85882**,
with 39 places remaining. Boarding station 97948 terminal 1 and alighting station
97294 terminal 0 matched. Original arrival bits `0x2275e0`, destination and waiting
place were preserved; the engine scope closed. Both worlds subsequently reported
assigned waiting state and then onboard state on S3. One engine attempt was logged.

User paused and saved `TPF3Interlining_after_switch_01`. Preserved copy:
`analysis/baselines/f1b92fb2ec15/TPF3Interlining_after_switch_01.sav`, **78,144,391**
bytes, SHA-256 `f1b92fb2ec1546ce305118464c6ab8f2faab796aedeba06f5efa93fef34146b1`.
Source modification time: `2026-10-08T10:52:06.310719+00:00`. Source size/mtime
were stable across copying and the copy hash matched.

One-second read-only snapshots before and after the save handoff reported the
same onboard state in both worlds: person 106847, S3, stops 1→2, vehicle 97913,
destination 85882. Both snapshots accepted four repeated-read samples and exited
3 because the original destination had not yet been reached. This is an onboard
checkpoint, not a post-arrival save. The snapshots did not alter game state.

Prepared the read-only observer under `analysis/native-bridge/reload-106847/`
using `--observe-person 106847 --observe-destination 85882`. Build passed with
warnings as errors; observer tracking tests and the actual DLL contract check
passed (capabilities 95, unsupported executable refused). The observer retains
a 3,600-callback limit and links no mutation backend. User was asked to restart
into the saved checkpoint, paused. Only `tpf_journey_observer.dll` is to be loaded
after that restart. Save/reload continuation was pending at this stage; the completed result is below.

| Artifact under `analysis/native-bridge/` | SHA-256 |
| --- | --- |
| `build-v6e/pre-save-106847.log` | `4a132ac00c508db8fc25331aef4213d1620a03e18696167ab0969dda3ecfa6a3` |
| `build-v6e/post-save-106847.log` | `4a132ac00c508db8fc25331aef4213d1620a03e18696167ab0969dda3ecfa6a3` |
| `reload-106847/tpf_journey_observer.dll` | `4c904a5ab5b04c9bb0f256a7af6660d638fead19f17976ddda885765b4592662` |
| `reload-106847/observer-validation.log` | `4c35883775d7c7688a112820b31fe6e7bb04dcba299d8d910e7b1d3135347e3a` |

## Reload started: saved onboard state preserved

User confirmed the game restarted and the after-switch checkpoint loaded paused.
Fresh Linux PID **44947**, Windows PID **304**, with no pre-existing bridge mapped.
Loaded only the recorded `reload-106847/tpf_journey_observer.dll`; `tfx_start=0`.
User unpaused at normal speed. New capture:
`analysis/native-bridge/reload-106847/tpf-bridge-304-7442601.log`.

Callbacks **1 and 2** reported onboard state for person **106847**, S3 (`97743`),
vehicle **97913**, stops **1→2**, original destination **85882**, in both worlds.
Thread **592**, world 1 **0x2e7aa690**, world 2 **0x2e7ab350**. This agrees with the
saved checkpoint and demonstrates persisted onboard route/vehicle/destination
state after restart without loading the mutation pilot. Journey completion after
reload was still pending at this stage.

The pre-restart mutation log is finalized by process exit, SHA-256
`85021f1a8550d3fd68563303a3c31fa3683499e1444ae4a7f63076791e274d64`.
Its last periodic record was callback 1,200, not a final callback count; the capture
was ended by the requested restart before its 18,000-callback limit.

### Onward transfer after reload

At callback 1,508 world 2 reported leaving onboard state; callbacks 1,509–1,510
reported onward movement in both worlds. At 1,689–1,690 both worlds reported
waiting on **S2 (`97742`), stops 2→3**, destination **85882** unchanged.
S2 stop 2 is Melksham (station 97294). A read-only external platform inspection
also read person 106847 at direct membership **97294:2**, consistent with that
transfer. This external scan was partial: world 1 was unavailable and world 2
was no longer idle at the end; it exited 3 with two errors. Treat the platform
read as corroboration, not an atomic snapshot or a replacement for the valid
callback observations. Log: `reload-106847/transfer-platform-106847.log`.

The user did not see an alighting passenger in the animation. We reported the
engine-state evidence and did not claim a visual verification. The user also
asked whether taking an earlier train to wait for the original train elsewhere
was new behavior. Clarified the implemented scope: it substitutes the passenger's
**existing adjacent train leg**, not necessarily their entire journey. Before
switching, this passenger was already booked on S1 stops 1→2 (Wimborne→Melksham).
The observed next service is S2, not that original S1 service. The complete
pre-switch itinerary was not captured, so whether S2 was already the original
next connection remains unverified. No trip-time improvement has been measured,
and no deliberate travel-ahead/wait-for-original-train policy was implemented.
A through leg beyond the shared segment remains excluded by the current pilot.

### Hook capture completed; external follow-up

The read-only observer completed 3,600 callbacks with both worlds observed,
destination mask 0, zero query errors/rejected boundaries and zero passenger
changes. Passenger 106847 remained waiting for S2 stops 2→3. External bounded
`journey_snapshot` follow-up then confirmed the same state in both worlds.
No hook was re-enabled and no new injection or engine change was performed.
Final arrival was still pending at the end of the hook capture.

Final observer log SHA-256:
`aaabc7d3906dba349840765a6055aa652a5a1427bfb35f7448543c6262ae6589`.
Partial transfer-platform log SHA-256:
`9364e4205388e622da59710571ce7f44d470fe11c8da89dcb0e482744a7a78ef`.

External follow-up 01 accepted 428 samples and ended while the passenger waited
for S2. Follow-up 02 accepted 439 samples and observed **onboard S2 vehicle 98390,
stops 2→3, destination 85882**, in both worlds. This boarding occurred naturally
with no mutation pilot loaded. A separate partial service snapshot during the
wait observed an S2 vehicle at stop 0; it did not establish a missed matching
boarding opportunity. Observation continued in follow-up 03. All snapshots are copied observations with repeated-read agreement,
not atomic transactions across worlds.

Follow-up 03 accepted 431 samples while the passenger remained onboard S2.
Follow-up 04 accepted 430 samples and recorded onward movement (`flags=4`),
vehicle/line cleared, original destination 85882 preserved in both worlds.
Follow-up 05 began in that onward-moving state. Those movement flags alone
do not establish walking mode or the exact S2 alighting location.

## Completed result and recovery verification

Follow-up **05** then recorded **at-building state (`flags=8`) at target/at-entity
85882 in both worlds**, with matching repeated reads. Its final summary is
`samples=413 destination_world_mask=3 observational_only=1`; helper exit **0**.
The wrapper recorded completion at **2026-10-08T11:19:57.272457+00:00** and stopped;
follow-ups 06–08 were not needed. No background watcher remains active.

This establishes the full observed sequence for person 106847: original S1
waiting leg → S3 boarding → onboard save/restart → restored S3 onboard state →
onward movement → S2 waiting and boarding → onward movement → original-destination
building. One engine change occurred before saving; all post-restart observation
was read-only. The hook observed 3,600 valid callbacks with zero query errors or
rejected boundaries. External observations used bounded copied reads and do not
claim an atomic cross-world snapshot. The two partial platform/service scans are
reported separately and do not weaken or replace the callback journey records.

The user reported an unexpected power cut after completion. Recovery verified
that final log and successful-exit metadata were present, both saved checkpoint
copies matched their recorded hashes and sizes, `git fsck --no-dangling` passed,
and `git diff --check` passed. Existing source milestones were already pushed;
only the final evidence/policy documentation needed completing. No test replay
or game-file repair was required.

| Artifact under `analysis/native-bridge/reload-106847/` | SHA-256 |
| --- | --- |
| `followup-01.log` | `e88a21dd88e5f0f5ec06b4c83adfa08524ed3e6e7ebdb829dc79a56fff2376a1` |
| `followup-02.log` | `810df5c0f95e2e53aa9006545a5b8c9147b911ee57094133f21d021a7b749284` |
| `followup-03.log` | `171fac88ec7ac74a5e2f4ec6d0fd43eade8c81f40351c7b7699477bf74b6b4c0` |
| `followup-04.log` | `b8ac3e4f7f0b106e27258ff6eed6b57eed1553e3b645e7cdc1b8ed6654f03b71` |
| `followup-05.log` | `884ef7510df8b3009611ef85ee7c72d7c8a8ea8571659eedab09b583701475af` |
| `transfer-service-check.log` | `41cc6536c44928808db0783a4100658a65d60e6ee1e61b08c7ac374a89eb4508` |

The result covers one saved onboard checkpoint and one native onward transfer on
the pinned game build. It does not establish arbitrary save compatibility,
repeated changes within one process, line-edit handling, multi-mod coexistence,
or improvements in travel time/fare/comfort. The complete original onward
itinerary is unknown. Next implement and validate a small bounded multi-passenger
trial while retaining fresh eligibility checks and preventing repeated selection
of the same passenger across worlds. Broader route/timing policy is separate work.
