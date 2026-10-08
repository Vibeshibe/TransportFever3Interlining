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
no old bridge mapped. These IDs must be rediscovered after restart. At this
checkpoint the build is prepared; live loading and outcomes are still pending.
