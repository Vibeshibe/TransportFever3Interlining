# Station-focused discovery — 0.9.1

## Problem and change

The stopped 0.9.0 trial completed three switched journeys but spent many checks
on unrelated stations: 1,172 of 1,489 screening attempts at callback 4,800.
These are repeated checks, not unique people. A larger waiting population did
not necessarily produce more useful candidates in the 32-person global cache.

Discovery now learns one passenger directly from each scanned page, rotating the
chosen position on successive completed sweeps. This avoids losing that person
to global-cache churn before learning their waiting station. A synthetic larger
population exposed repeatable omissions in the previous cache-based learning.

Copied boarding-stop queries populate a bounded index of 16 station groups,
with up to 32 passenger/line hints per group. For a loading vehicle, the first
three screening positions prefer that station and another line. The fourth
position, if reached, explores the global cache; empty buckets also fall back.
Round-robin selection and eviction admit later passengers. A callback never
screens the same person twice.

Hints only choose who to inspect. Waiting state, boarding station, full route,
platform access and capacity are read again, followed by deferred revalidation
before the existing native operation. Moved hints are relocated; missing,
assigned and selected passengers are forgotten. No engine pointers are cached.

## Bounds and evidence

The prepared build retains 32 lifetime attempts, four concurrent journey samples,
one pending boarding and an 18,000-callback cap. Discovery still scans at most
1,024 masks, checks four vehicles and four passengers, and fully validates at
most one candidate per callback. Station learning adds one copied query; the
conservative worst-case fixture uses 63 of the 64 permitted queries.

Tests cover station migration, stale/missing records and recovery, cache eviction,
same-line exclusions and duplicate suppression. The crowded consumer fixture
varies the sole matching passenger across all 80 positions, warming discovery
for 128 callbacks with no loading train, then checking selection on the first
loading callback. It runs in normal, reserved and sampled configurations. Legacy
lifecycle, route, boarding and capacity tests remain required.

`pilot_discovery` adds `indexed_picks`, `fallback_picks`, `hint_station_mismatch`,
`indexed_people` and `indexed_stations`. Counters measure repeated work, not queue
sizes. The 32-entry station window is not a complete list of an 80-person queue;
this change does not guarantee every eligible passenger boards or remove the
existing route/platform restrictions. Live improvement remains unmeasured.

## Next trial

Prepared artifacts belong in ignored `analysis/native-bridge/build-v9b`.
Restart the game because the stopped 0.9.0 DLL stays mapped until exit. Load a
disposable `TPF3Interlining_before_multi_passenger_01` and leave it paused for
process, save and artifact verification. No additional diagnostic reload is
planned. Compare station mismatches and indexed selections, then boarding and
sampled outcomes. Keep the bounded capture; do not extend it just to fill a cap.

## Final offline validation

Warnings-as-errors build and all 13 native test invocations pass. The 80-position
fixture passes in normal, reserved and sampled consumers; the existing 32-attempt
fixture still confirms four samples and 28 released boarding trackers. Worst
query allowance is 63/64. No new live outcomes are claimed.

Prepared artifact SHA-256 values:

- `tpf_shared_bridge.dll`: `a21d9fa8548c43c91be01d4210aff43de7295c872ecc233e3d40c804fd32cacb`.
- `tpf_journey_observer.dll`: `a9a6070094cc2d7c3e91d61dd53f42e12c5c8fb316b112c37accf9c745d1d407`.
- `capacity-tests.json`: `dfa2b4064fb4b0a2d2bfe5b0fb5f64312508ad5af1bbb2c18a9a8e63b8034cb6`.
