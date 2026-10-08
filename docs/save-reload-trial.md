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

Runtime repeat and save/reload results are pending.
