# Discovery coverage — 0.11.0

## Problem

The paused 0.10.1 trial contained twelve S3 passengers waiting for Faringdon at
Wimborne, but only one was in the station hint cache. The sixteen-vehicle cache
also omitted an S1 train at Wimborne. Only sixteen stations could be retained,
so discovering bus stops could evict a rail station's entire queue. These are
confirmed coverage gaps; the exact cache contents during the two missed S2
departures were not captured. See [live evidence](return-terminus-fix.md).

Previously, discovery read one waiting leg from each scanned page, then advanced
to the next page. It revisited the other people only on later world sweeps.
Dense queues and sparse entity pages therefore took many sweeps to index.

## Implementation

Each returned page is now retained and drained over successive discovery
callbacks: one fresh waiting-leg read per callback, then its boarding-station
hint. The next page is scanned only after those IDs have been consumed. Deleted,
assigned and already switched people remain excluded; cached IDs never authorize
a change. This still scans at most one bounded mask page per callback.

Search storage now holds 64 station groups with 128 people each, 64 demand lines
and 128 vehicle IDs. The 32-person global fallback and bounded replacement remain.
These are search hints, not full passenger journey trackers.

Evaluation checks up to four vehicles and four passengers, with at most one full
candidate validation. Vehicles observed in the accepted loading state enter a
sixteen-ID priority queue. Evaluation alternates its first choice between that
queue and the global vehicle sweep. Queued vehicles are re-read before use.
Loading-index membership alone does not put a vehicle in this queue: a train
whose boarding window has closed must not crowd out a ready train.

If a callback performs no full candidate validation, it can use up to twelve
additional vehicle reads to find ready vehicles for later evaluation. These
reads never accompany staging or deferred revalidation. All route, platform,
capacity, native loading-state and lifecycle guards remain. ABI 8 and the
engine-owned waiting-leg change are unchanged.

`pilot_ready` records up to 128 observed boarding opportunities with vehicle,
line, stop, station hint count and free places. Periodic metrics add drained
pages, pending page IDs, priority selections and additional vehicle reads.

## Validation

The new `discovery_tests` fixture has 241 people, an 80-person rail queue,
forty competing bus stations, 81 vehicles and one hundred empty scan pages.
It verifies that every person and vehicle is discovered, then exercises sixteen
different timings of a short loading window, repeated distinct switches and
closed-window/deleted-vehicle priority hints. The fixture uses arbitrary IDs,
cross-platform boarding and return-to-first-stop legs.

Existing tests cover routing, deferred route edits, capacity, lifecycle outcomes,
32 attempts with four concurrent journey samples, host limits and DLL contracts.
All fourteen final native invocations passed on 2026-10-11. The new fixture
peaks at 44/64 queries; the existing worst-case fixture remains
63/64. Larger station queues and cache overflow still have explicit bounds.

Build the experimental trial:

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v11a \
  --passenger-limit 32 --journey-sample-limit 4 --observation-limit 18000 \
  --observe-person 106847 --observe-destination 72810
```

The optional observer target is a contract-test artifact and is not included in
the switching package. Load only `tpf_shared_bridge.dll` using the
[native loader guide](../native/README.md#explicit-loading-under-windowsproton).

## Integrated trial and limits

Restart before loading 0.11.0 because the paused game still contains 0.10.1.
Use a disposable `TPF3Interlining_before_multi_passenger_01`; verify the fresh
process, actual save and fingerprints, then load while paused. Observe S2 toward
Faringdon at Wimborne, its station-hint coverage and boarding opportunities.
Check confirmed boarding separately from original-destination completion.

The trial retains 32 attempts, four journey samples and 18,000 callbacks. This
is bounded sampling, not complete instantaneous enumeration. Cold startup,
larger networks, mixed directions, short dwell times and the one-pending-boarding
rule can still limit throughput. Offline results do not prove every eligible
passenger will board in the game. Partial live evidence follows.

### Live checkpoint — 2026-10-11, callback 2,400

Fresh baseline/process verification and loading succeeded; the user confirmed
normal-speed unpause. Twelve distinct changes and twelve both-world boardings
are confirmed. Five passengers received detailed journey samples: one reached
the original destination, one resolved home invalidation (not an arrival), and
three remain active. Seven boardings were unsampled; their final arrivals are
unknown. No query errors or stop marker were logged at this checkpoint.

The user-reported S2 departure from Wimborne toward Faringdon produced a confirmed
S1→S2 switch: person 108249, vehicle 98388, staged at 1275, boarded in both worlds
at 1277. Both boarding and fixed alighting platforms differ; the planned leg
remains Wimborne→Faringdon, stops 5→0. There were 35 free places at validation.
This passenger was unsampled. Only one switch is confirmed for that departure;
the user's reported 21 S1/S3 waiters were not all individually validated.

The implementation does not compare the following S1 train's arrival time.
One full candidate validation per discovery callback, pauses for pending
boarding, mixed-direction hints and short loading windows can limit throughput.
The exact bottleneck for this departure has not been isolated. An earlier
user-reported missed S3 opportunity had no matching ready log; later cache
contents cannot reconstruct that loading window.

At callback 600, discovery retained 145 station hints versus 58 in the earlier
0.10.1 run. At 2400 it retained 241 hints across twenty stations and had performed
fifty validations. Hints are not proven eligible passengers. Person 115753
reached original destination 75525 in both worlds at 2136–2137 after the changed
train leg and a native bus connection. The trial remains incomplete.

Raw evidence is ignored under `analysis/native-bridge/build-v11a`:
`tpf-bridge-312-7380832.log`, `live-run.json`,
`early-discovery-snapshot.json`, and `s2-faringdon-checkpoint.json`.

## Prepared artifacts

Ignored build: `analysis/native-bridge/build-v11a`. The trial ZIP contains the
mutable DLL, loader, fourteen-case report, instructions and source/hash manifest.
The read-only observer is excluded. No persistent installation is performed.

- DLL SHA256: `4e8cf8b356e9cfef465064c31838a67ba78d2a812acd199367087f910d9c8952`.
- Report SHA256: `88dcc3774f6861f747647cdb05807aedb87bf894c8eba9116d49c8c9871dbb4f`.
