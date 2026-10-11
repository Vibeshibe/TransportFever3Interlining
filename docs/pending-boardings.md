# Concurrent pending boardings — 0.14.0

Status: implemented on `dev`; all 18 offline checks pass, live validation pending. The current
0.13.1 game remains unchanged. This candidate targets boarding throughput and
retains continuous discovery, original destinations and the existing route policy.

## Problem and behavior

At Wimborne, 0.13.1 switched two S3 passengers and one S2 passenger to S1 toward
Faringdon, but the user reported people remaining in both queues. The first
passenger took twelve callbacks to confirm boarding; the single-pending gate
prevented other switches during that wait. This explains a measured delay,
not every remaining passenger or missed departure.

Continuous builds now permit **four pending boardings** across all vehicles.
Each dispatch still stages at most one freshly validated change. Results are
collected before another request: an uncertain or failed mutation blocks later
changes. A stale request rejected before any engine attempt can be discarded.

Each unconfirmed passenger holds one place against that vehicle's copied free
capacity. Selection and deferred revalidation both subtract other pending holds;
the request being revalidated does not subtract itself. Holds release only after
boarding the expected train and leg is confirmed in both simulation worlds.
Native reservations may already include a passenger, so this can temporarily
underuse seats; it deliberately does not assume those reservations are absent.

## Processing and failure handling

Paired tracking phases alternate with candidate-selection phases. At most four
full journey samples and four pending passengers are tracked. Discovery pauses
while pending boardings or result reads occupy its query allowance. All existing
64-query, copied-read and byte limits remain; no loop scans every waiting person
in a single update. Failed results are checked even on selection phases, and each
pending passenger retains its exact 600-callback confirmation deadline.

Unexplained identity, destination or boarding changes still stop switching.
Lifecycle uncertainty freezes new proposals. Episode duplicate guards, native
state comparison, route checks and ABI9 remain unchanged. Bounded research builds
retain one pending boarding. Continuous mode has no lifetime switch/callback cap.

## Reproducible checks

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v14a --continuous \
  --observe-person 106847 --observe-destination 72810
rtk proxy python3 tools/test_native_bridge.py \
  --build-dir analysis/native-bridge/build-v14a
```

`pipeline_tests.exe` uses the real consumer/policy with mocked game queries. The
same delayed-boarding fixture stages one passenger with the exact 0.13.1 consumer
and four with 0.14.0 in twelve callbacks, before any boarding confirmation.
Regressions cover partial replica confirmation, capacity shrinkage, failed and
pending results, canceled retry, exact deadlines, wrong vehicle/destination and
four journey samples alongside four pending boardings. Peak usage is 64/64 queries.
The comparison is stored under ignored build-v14a; it is not a live throughput claim.

## Next live trial and release limits

Restart into the same disposable baseline, load the prepared build, then observe
Wimborne→Faringdon departures serving S1/S2/S3 queues. Check multiple pending
boardings, confirmations, sampled original destinations, callback costs and
continuation beyond 32 changes. The old DLL remains loaded until game exit.

Queue clearance is not guaranteed. Four-stop route scope, alternative-alighting
exclusions and bounded discovery still apply. Line edits/new services, later
journeys, save/restart and game responsiveness remain stable-release gates.
Automatic installation and save-session reset are not included. Keep `main`
unchanged until the release evidence supports promotion.

## Final offline evidence

All 18 suite invocations passed against fingerprint-verified final binaries.
Continuous stress staged 5,185 requests in 66,500 callbacks, covering recycled
records, later episodes and new/deleted service; host stress covered 4,096 episodes.
The native copied-query benchmark measured a sparse-page mean of 3.141 µs and
a 64-waiting-query mean of 190.295 µs (p95 280.700 µs, maximum 379.200 µs).
These synthetic Wine measurements do not establish in-game responsiveness.

- `tpf_shared_bridge.dll`: `a9649f3bfa4f9885d30d21bfa8513af53ee9dab39ae157040de4aeac1bee915f`
- `capacity-tests.json`: `f596db431dc8b2f968814d98f3e7e1b51a468f4290a02a8fea7e430d168ac3a2`
- `pipeline-comparison.json`: `1cb40519ea57caa9844261db39148ec62138cb46281bdd7aaaa0500fbd1350cc`
