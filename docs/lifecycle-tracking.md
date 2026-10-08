# Lifecycle-aware journey diagnostics — 0.7.2

## Behavior and limits

The [read-only control](passenger-lifecycle-investigation.md) reproduced passenger
removal and workplace invalidation without interlining. Version 0.7.2 makes these
states distinguishable in the bounded pilot instead of reporting only missing
journey data or an unexpected target. The completed live trial below confirmed
five boardings, four original arrivals and one separately classified removal.

The shared bridge adds a copied lifecycle query through ABI 7; ABI 1–6 remain
compatible. Before staging a change, the consumer records the current home and
the slots containing the original destination. A malformed, missing or unassigned
baseline prevents the attempt. The new query checks component row/mask agreement
and the three residence/shopping/work entries, using existing memory/query budgets.

| Outcome | Evidence required | Effect |
| --- | --- | --- |
| `original_destination_reached` | Expected first boarding, then at-building original target in both worlds | Arrival masks become3; later trips do not undo arrival. |
| `passenger_removed` | Valid snapshots without SimPerson in both worlds | Stop with a removal classification; no added arrival. |
| `destination_invalidated` | Original slots cleared to-1, original destination absent from all slots, unchanged home, in both worlds | Stop with invalidation classification; no added arrival. |
| `unresolved` | Read/identity errors, changed home, reassignment, disagreement or incomplete evidence | Block further switching; stop immediately for semantic conflicts or at existing error/callback bounds. |

The **first anomaly permanently blocks new proposals** in this capture, including
an error that later recovers. Further callbacks only gather diagnostic evidence.
Matching lifecycle evidence in both worlds ends the pilot. A non-person entity
at a reused ID is not a passenger. Reappearance or restored assignment after an
observed loss in the same world is unresolved. Existing cap of five distinct
attempts, one engine attempt per dispatch, fresh revalidation, duplicate protection
and the S1↔S3 adjacent-leg policy remain unchanged.

Classification describes observed state. It does not prove a natural cause or
provide a stable generation ID; unchanged home alone cannot establish identity
under arbitrary ID reuse. Do not use these outcomes as permission to resume
mutations. `pilot_lifecycle` logs include person, original destination, callback,
world mask and confirmation. Final passenger summaries include separate arrival,
removal and invalidation masks. The summary records `switching_blocked`.

## Build and validation

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v7c \
  --passenger-limit 5 --observation-limit 18000 \
  --observe-person 122261 --observe-destination 72934
```

The separate optional observer is for ABI contract checks here; the next switching
trial loads `tpf_shared_bridge.dll`. Build succeeded with warnings treated as errors.
The following isolated Wine tests passed:

- `query_tests.exe`: valid/absent/reused passenger records, cleared destinations,
  malformed vectors, row/mask disagreement, shared page31, output preservation,
  budget/thread/context guards; prior query and mutation regressions.
- `pilot_tests.exe`: both-world classifications, delayed replica confirmation,
  contradictory observations, reassignment/home changes, failed/recovered reads,
  preserved arrivals and no subsequent proposals. Maximum fixture allowance40/64.
- `host_batch_tests.exe` and separate `failure` invocation: existing attempt,
  deduplication, conflict and failed-postcondition limits.
- `bridge_tests.exe`, `candidate_tests.exe`: hook/registry and route-policy regressions.
- `journey_probe_tests.exe DLL`: observer binary reports API6/7 capabilities95/223;
  mutable binary with `--mutable` reports127/255. Both reject unsupported executables.

Final log-only change was followed by a consumer regression and rebuilt mutable
DLL contract check. Test commands and outputs are preserved in ignored
`analysis/native-bridge/build-v7c/lifecycle-test-results.json` and
`lifecycle-final-checks.json`; adjacent `lifecycle-hashes.json` records SHA-256.

Final mutable DLL:
`2f8ca09d693e9411327660a730e4278e17347e012e65cc0a4a0f3da3e3127ca0`.
Observer DLL:
`08a0273f59d6bfa0afd01bceed523f5d3aefde73f8a285fad936834c40c333aa`.

## Live trial procedure

1. Restart the game: the previous observer remains loaded until exit. Load a
   disposable copy of `TPF3Interlining_before_multi_passenger_01` and leave paused.
   This is the restored expanded pre-trial baseline, SHA-256
   `ce2e02d935da822e63fbd8a12fe24234cfef6a5698d841e4f978939ca3c80f84`.
2. Verify the fresh game process and save, then explicitly load the new mutable
   DLL using its loader in the game's Proton prefix. Record PID, log, world
   addresses, DLL hash and successful startup. Never reuse the control's addresses.
3. After successful loading, ask the user to unpause at normal speed. Capture
   up to18,000 callbacks, with at most five attempts; avoid line/vehicle edits.
4. Check expected boarding and per-passenger outcomes. If lifecycle events recur,
   verify both-world classification, no false arrival and no later proposal.
   Unexplained failures remain investigation stops. Capture/save only if needed;
   preserve the pre-trial and earlier diagnostic backups.

This trial validates the new classification, not unrestricted continuation after
retirement, broader route eligibility or a stable release. At the implementation checkpoint the DLL was not yet loaded; the startup and
completed outcome are recorded below.

## Live startup — 2026-10-08

Implementation milestone `935ef65` is pushed. User restarted, then corrected the
initial save selection before any injection. Game stdout confirms loading the
preserved `TPF3Interlining_before_multi_passenger_01` at14:49:51Z and ready at
14:50:46Z. Source hash matches the baseline above. Fresh process identity and
absence of previous bridge mappings were checked before loading.

Linux64386 / start1139296, Windows308; `tfx_start=0`. User confirmed unpaused at
normal speed. Log `build-v7c/tpf-bridge-308-11756348.log` confirms version0.7.2,
cap5/18,000, lifecycle stop enabled, thread776; worlds1 `0x6748b7d0`,2 `0x6748ab10`.
These addresses are valid only for this load/process. The capture began normally;
no live classification or boarding outcome was claimed at startup. Metadata and watcher
are ignored artifacts beside the build. Preserve the loaded DLL unchanged.

Interim progress (superseded by the completed result below):120842 staged512/boarded514,122261 staged518/boarded520, both S3→S1
vehicle97745. Person40365 staged1074/boarded1095, S1→S3 vehicle97912. Both-world
alighting observations place replacement vehicles at stop2. Person122261 then
used line79460/vehicle123296 and reached original destination72934 at2444/2445.
Unlike earlier trials, original arrival preceded any observed workplace change;
subsequent lifecycle changes must not invalidate that completed journey. Other
outcomes were pending at that checkpoint.

## Completed live result — 2026-10-08

The pilot stopped at **callback 7670** with `reason=passenger_removed`, five
staged passengers, zero query errors and `switching_blocked=1`. The final log
confirms the wrapper forwards only. Watcher exited0 at15:17:46.912452Z after
1,499.36 seconds of watching; this is wall time, not simulation time.

| Person | Replacement | Vehicle | Stage / both-world boarding | Original destination | Final outcome |
| --- | --- | --- | --- | --- | --- |
| 120842 | S3→S1, stops1→2 | 97745 | 512 / 514 | 78804 | Removed before confirmed arrival,7670 |
| 122261 | S3→S1, stops1→2 | 97745 | 518 / 520 | 72934 | Original arrival2444/2445 |
| 40365 | S1→S3, stops1→2 | 97912 | 1074 / 1095 | 86033 | Original arrival4593/4596 |
| 72701 | S3→S1, stops1→2 | 98594 | 4698 / 4706 | 72691 | Original arrival6854/6855 |
| 73064 | S3→S1, stops1→2 | 98594 | 4711 / 4713 | 73063 | Original arrival6899/6900 |

Every engine result was status0/attempted1, with original arrival-time,
original-destination and waiting-place bytes preserved. Every substituted leg
boarded in both worlds before the next proposal; all five alighted with their
replacement vehicle at planned stop2. Native onward services were observed:
120842 continued on S1/98610,122261 on79460/123296,40365 on S2/98301,72701 on
79460/123294 and73064 on79461/123293. These are journey observations, not proof
of a faster route or the unmodified counterfactual itinerary.

120842 left its onward train at planned stop3 at7498/7499 and was moving toward
78804 at7500/7501. At7669, world1 reported missing SimPerson;7670 confirmed the
same in world2. Its arrival mask stayed0 and removal mask became3. Final repeated
external inventories corroborate that120842 now has only an EcsOctreeNode
component; original home75531 and destination78804 remain buildings. This is a
new removed passenger compared with the earlier control. **Its specific removal
cause was not traced**, and the earlier120770 control does not prove that cause.
No arrival is inferred from disappearance.

An independent read-only snapshot after122261's arrival found target122255 and
slots[122255,75501,-1] in both worlds. The final tracker summary correctly kept its
original arrival mask3 despite subsequent workplace invalidation. This is live
coverage of preserving completed journeys, not of the pre-arrival invalidation
stop branch; that branch remains fixture-tested only.

The audit checked five distinct requests/results, preserved fields, both-world
boarding before each next request, the attempt/callback bounds, confirmation
masks and no requests after removal. The five-attempt cap was already exhausted
before removal, so the live absence of later requests alone does not isolate the
new blocking logic; the consumer fixtures cover that with remaining allowance.
All logged boundary checks had rejected0. No six-passenger run, broad route policy,
full batch arrival, natural cause attribution or stable release is claimed.

Artifacts under ignored `analysis/native-bridge/build-v7c/`:

- `tpf-bridge-308-11756348.log`, SHA-256
  `64ff0426fc8bb1bd8514639fd263c4fa91d475af295e939679ac87e1203c5f69`.
- `after-arrival-lifecycle.json`, SHA-256
  `2fc406de8b4ff44723320f16958607e0f8a50f5d14eaa21ce43ec129ea07e83d`.
- `final-entity-inventory.json`, SHA-256
  `923e3718e268956957b816a03ffaed8c28676b8c2d6d05c1a22ff4d0090794c7`.
- `live-audit.json`, `audit_trial.py`, `live-run.json`: checks and process metadata.
- `last-vehicle-progress.jsonl`: bounded observational vehicle-state corroboration.

External inventories are repeated copied reads with closed-window checks, not
atomic transactions. All external readers/watchers have exited. User was told
capture is complete and may pause the game. The DLL remains loaded, forwarding
only, until exit. No new save checkpoint was requested or claimed.

The diagnostic milestone is complete for removal classification and preserving
arrivals through later invalidation. Further implementation should keep unknown
identity/query failures as hard stops and handle classified lifecycle outcomes
explicitly; do not silently turn them into successes or loosen route eligibility.

The later [version0.7.3 continuation implementation](lifecycle-continuation.md)
changes the global stop after classified outcomes while preserving failure
blocks and attempt bounds. The results above remain specific to0.7.2.
