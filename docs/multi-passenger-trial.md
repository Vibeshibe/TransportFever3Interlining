# Bounded multi-passenger trial — 2026-10-08

## Setup and purpose

The user progressed the baseline, improved signalling and added feeder buses,
reporting 40+ waiting passengers. The trial increases the operation count while
retaining S1↔S3 original stops 1→2 (Wimborne→Melksham), same terminals, existing
adjacent leg, ready vehicle, permitted loading/unloading and fresh capacity checks.
Bus switching, platform changes, longer legs and preference/time optimisation are
outside this trial. Do not shorten through journeys to manufacture transfers.

Preserved `TPF3Interlining_multi_passenger_01.sav` under ignored
`analysis/baselines/ce2e02d935da/`; **80,431,012 bytes**, SHA-256
`ce2e02d935da822e63fbd8a12fe24234cfef6a5698d841e4f978939ca3c80f84`.
Source mtime **2026-10-08T13:34:55.240083+00:00**; source size/mtime stable during
copy and backup hash verified. Original and onboard checkpoints remain intact.
The user confirmed restart into a disposable copy, paused.

## Implementation and bounds

Version **0.7.0**, ABI **6** unchanged, explicit build inputs:

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v7a \
  --passenger-limit 5 --observation-limit 18000 \
  --observe-person 106847 --observe-destination 85882
```

The optional observer was built for its binary regression test, not this live
trial. Load only `tpf_shared_bridge.dll`. Its SHA-256 is
`fce3ebbef7f36ceef06b94fdfe9887c2463bdeb5d49c7ec8a120204d252e1051`.
No automatic installation or stable release is supplied.

Host: 1–5 distinct person attempts per DLL lifetime (default one), at most one
engine attempt per dispatch, ID ledger shared across worlds/clients, no repeat
attempt for a replicated person. Conflicting staged requests still all reject.
Each accepted request receives fresh policy and complete raw-state validation.
Any attempted operation returning failure latches all later host writes shut;
there is no rollback guarantee. The native Begin/Remove/Add/End adapter is unchanged.

Consumer: at most five staged passengers and one unconfirmed boarding. Before the
next switch, confirm the expected vehicle, line and stops in both worlds. Track
original destinations separately, including native onward transfers. Selected
IDs are excluded from discovery even if old waiting data remains in a replica.
Tracking and discovery have separate callbacks; discovery performs only one full
validation plus fresh host revalidation. Existing 64-query/read/byte budgets remain.
Stop on final callback (without a new proposal), all five arrivals, failed result,
unexpected boarding/destination/world, or eight query errors. Report per-person
world masks even for incomplete captures. Already observed arrivals are not
confused with later trips by that person.

## Offline validation

Warnings-as-errors build succeeded. Isolated Wine tests passed:

- `bridge_tests`: registry, callback lifetime/thread guards and hook ownership.
- `query_tests`: native copied layouts, mutations, fifth add argument, complete
  value preservation, postconditions and default one-attempt host.
- `candidate_tests`: route/terminal/platform/capacity gates and rolling discovery.
- `host_batch_tests` plus separate `failure` process: distinct-person allowance,
  cross-world/client deduplication, one attempt per dispatch, conflicts, fresh
  revalidation, cap and fail-closed postcondition handling.
- `pilot_tests`: real policy/consumer against SDK doubles; delayed two-world
  boarding, five distinct people, fresh capacity rejection/recovery, failed
  result, unexpected boarding/destination, eight errors, cap and destination
  completion. Fixture query allowance peaked at 39/64; this is not a native
  performance measurement. Final-callback no-new-proposal regression also passed.
- `journey_probe_tests PATH_TO_OBSERVER_DLL`: read-only capabilities 95,
  unsupported executable refusal and existing observer behavior.
- Build helper rejected passenger caps 0, 6 and -1. `git diff --check` passed.

Raw test output: ignored `analysis/native-bridge/build-v7a/offline-tests.log`;
final consumer regression was rerun after adding the no-new-proposal cap guard.

## Live validation

Preflight located fresh Linux process **26846**, Windows process **312**, with
no old bridge mapped. Implementation committed/pushed as `cb7c91d`; startup
checkpoint `a19a76b`. Loader returned **`tfx_start=0`** and the user unpaused at
normal speed. Fresh world1 `0x2d1de790`, world2 `0x2d1df450`, simulation thread592.
Never reuse these addresses after restart/reload. The user later confirmed **no
vehicle or line edits during capture**, then paused for a diagnostic save.

### Four successful replacements and boardings

All four requests reported status0/attempted1, complete age/destination/place
preservation and closed engine scopes. Each boarded the proposed vehicle and leg
in both worlds before the next request was permitted:

| Person | Staged callback/world | Change | Vehicle | Destination | Free places | Both-world boarding |
| --- | --- | --- | --- | --- | --- | --- |
| 120842 | 418 / 2 | S3→S1, stops1→2 | 97745 | 78804 | 36 | 425 |
| 122261 | 427 / 1 | S3→S1, stops1→2 | 97745 | 72934 | 29 | 429 |
| 120770 | 1074 / 2 | S1→S3, stops1→2 | 97912 | 72649 | 18 | 1088 |
| 122790 | 1478 / 2 | S1→S3, stops1→2 | 97914 | 86009 | 27 | 1480 |

Boarding station97948:1, alighting station97294:0 throughout. This is the first
observed multiple-person change sequence in one process, with writes originating
in both worlds and no repeated passenger. Two passengers used the same train with
fresh capacity readings. Compatible return-direction proposals remained outside
the pilot's old-stops1→2 filter; longer/incompatible journeys were not shortened.

### Automatic stop and incomplete destination evidence

After callback2118, journey reads returned **TFX_NOT_FOUND (6)** eight times.
The consumer ended the run with **staged=4**, remembered32 people/12 vehicles,
80 ready samples,43 validations,5 generic candidates and8 query errors.
Four per-person summaries have onboard mask3, destination mask0. The fifth
generic candidate was outside the pilot leg filter; it was not a fifth switch.
Periodic boundary logs through2100 show rejected0;2100 is not the final callback.
The old error log omitted person/sequence, so the exact stop callback and error
owner are not directly recorded. The bridge explicitly disabled observations and
future changes; its pinned wrapper still forwards until game exit. No fifth
attempt or replacement DLL was loaded.

Read-only snapshots narrowed the missing journey to **120770**. In both worlds,
its entity and destination72649 still have components, but known person, waiting,
onboard, moving and at-building types are absent from both its component mask
and row handles. Idle pre/post checks passed. This explains the query rejection,
**not the cause of the component removal or its relation to the switch**.
Do not classify the missing record as an arrival, normal retirement, deletion of
the entire entity, or mod corruption without further lifecycle evidence.

Native continuation for the others:

- **120842:** left S1 at stop2 at1609–1613; waited on S1 stops4→3 at1777–1780;
  external reads later found onboard S1 vehicle98610 with destination78804.
- **122261:** left S1 at stop2 at1613–1617; waited on line79460 stops0→1,
  boarded vehicle123296 at1857–1860, left at2080–2085. External follow-up found
  at-building flags8 in both worlds, but reported target/at-entity **122255**,
  not original destination72934. That mismatch remains unresolved: no original
  destination arrival or building-ID equivalence is claimed.
- **122790:** external reads found native S2 stops2→3, then boarding vehicle98301
  in both worlds, destination86009 retained.

The three 120-second external windows all exited3 (original destination not
confirmed), with accepted samples439/430/432 respectively. They are copied reads
with repeated agreement and idle endpoints, not atomic cross-world snapshots.
All watchers exited. **Four boardings are proven; five-person completion and
end-to-end destination correctness of this batch are not.**

### Preserved diagnostic checkpoint

The user confirmed paused/saved and no vehicle or line edits. The game wrote the
checkpoint under the existing `TPF3Interlining_multi_passenger_01.sav` name.
Copied it as `analysis/baselines/acca331ade37/TPF3Interlining_multi_passenger_diagnostic_01.sav`;
**81,236,125 bytes**, source mtime **2026-10-08T13:59:39.503645+00:00**,
SHA-256 `acca331ade37c604be598cd694e28cf15fc7e8bdef25ff5b71ae6e7be5b86219`. Source size/mtime remained stable during copying and the
backup hash matched. The earlier ce2e02d935da pre-trial backup remains intact.

### Diagnostic improvement and next gate

Version **0.7.1** adds person/destination/sequence to consumer journey errors and
adds read-only component presence/mask/row diagnostics to `journey_snapshot.exe`.
It does not loosen the missing-record stop or mutation policy. Built separately
under `analysis/native-bridge/build-v7b`; its DLL was **not loaded**. The consumer
regression including person-specific error logging passed. The standalone helper
built with warnings-as-errors and its live read-only diagnosis is preserved under
`analysis/native-bridge/diagnostic-v7a/`.

Next investigate the disappearance of simulation-person state and the differing
at-building ID before repeating/expanding the mutation trial. Use the expanded
baseline and diagnostic checkpoint; preserve the failed-observation evidence.
Compare native lifecycle behavior and target/building identity, rather than
silently treating either case as successful arrival or bypassing the stop.

### Evidence hashes

Generated artifacts are ignored, with load metadata in `build-v7a/trial.json`
and hashes in `build-v7a/evidence-hashes.json`. SHA-256:

- `analysis/native-bridge/build-v7a/after-stop-120770.log`: `22c1bdff94aeb66a00efe8b50d6886cfa6741bacea2a38cec1a3f2b38f5280dc`.
- `analysis/native-bridge/build-v7a/after-stop-120842.log`: `8f3cc2e30990d225e819607280d26e9b4655dc00e065e7a11cb712af338c72cb`.
- `analysis/native-bridge/build-v7a/after-stop-122261.log`: `7184b590c35faab2fc817ca72af495710a4ed693a4f2a472b9f07d5730e71604`.
- `analysis/native-bridge/build-v7a/after-stop-122790.log`: `dd439b2bd9c40cc5f5bc4cd3955ceb1df5385913e2639acfaba26ff1e4fe1c5a`.
- `analysis/native-bridge/build-v7a/followup-01-120842.log`: `eb798fd4169eb55c48d873a91a3664df196f31435bd31f7bbea26d51507cc28f`.
- `analysis/native-bridge/build-v7a/followup-01-122261.log`: `b5c6473e5d4e4833e6c75167147dacf061a2a1876d42eccdaba9812f0c4c8b6c`.
- `analysis/native-bridge/build-v7a/followup-01-122790.log`: `2c4bec4c5d56ba4cef77a9e170ed4b8892db546ed33398354c88771f1d3d7af5`.
- `analysis/native-bridge/build-v7a/tpf-bridge-312-7795382.log`: `3a0d2f1908953b354bf6ced657753401aa25035e30d7c8f4ecc6b32f79cc3888`.
- `analysis/native-bridge/diagnostic-v7a/missing-person-120770.log`: `917189342557875811962648b84579243b2a3c23f11a4daf3a7c927d3fba795a`.
