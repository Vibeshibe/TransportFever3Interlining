# Lifecycle-aware journey diagnostics — 0.7.2

## Behavior and limits

The [read-only control](passenger-lifecycle-investigation.md) reproduced passenger
removal and workplace invalidation without interlining. Version 0.7.2 makes these
states distinguishable in the bounded pilot instead of reporting only missing
journey data or an unexpected target. Live validation of this revision is pending.

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

## Prepared live trial

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
retirement, broader route eligibility or a stable release. No new DLL has been
loaded into the game at this implementation checkpoint.
