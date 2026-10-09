# Bounded shared-route expansion — 0.8.0

## Behavior

A passenger waiting for a local A→B→C service may now board another A→B→C
service, or an express A→C, for the same planned alighting stop C. This replaces
the fixed S1↔S3 stops1→2 rule. The original leg may span one to four forward stop
intervals; both endpoints must use the same accessible terminals as before.
The final destination and waiting age/place remain unchanged. Native onward
routing still runs after alighting.

| Planned leg | Substitute | Decision |
| --- | --- | --- |
| A→B→C | A→B→C | Eligible if platform/loading/capacity checks pass |
| A→B→C | A→C | Eligible express substitution |
| A→B→C→D | A→C→D | Eligible ordered subset |
| A→B→C→D | A→C→B→D | Reject reversed station order |
| A→B→C | A→B→X | Reject branch; never shorten the leg to B |
| A→C | A→B→C | Excluded: extra stops are outside this version's policy |
| A→B→C | Different platform at A or C | Excluded |

Original legs with repeated station groups, loop wrapping or more than four
intervals remain excluded. A replacement must follow an ordered subset of the
original station groups through the original alighting group. The first visit
to that group must match its terminal; a later visit cannot repair a mismatch.
This checks station order, not physical track direction or whole-trip duration.
It does not establish that every earlier departure produces an earlier arrival.

## Discovery and revalidation

The consumer learns line IDs from waiting passengers, rather than hard-coded
save entities. Each discovery callback scans at most 1,024 masks, remembers up
to 32 passengers, reads one remembered waiting leg, then samples one stop on one
of up to 16 remembered lines. Independent stop cursors advance each line's scan.
Up to 16 vehicle IDs are retained. Full caches replace old IDs; deleted entries
are reusable. Already attempted passengers remain excluded.

This is demand-based sampling, not complete network enumeration. A service with
no observed waiting passenger may remain undiscovered, and short boarding windows
may be missed. Cached IDs never authorize a write. Every relevant stop, platform,
vehicle, destination, permission and capacity is queried again during validation.
Absent discovery entities are skipped; malformed reads retain the error guards.

Before staging, the consumer retains copied original and replacement leg records.
The deferred validator compares those copies exactly after reading fresh data.
A route edit rejects the request even when the edited routes still match each
other at unchanged indices. Removal, shrinking, branch divergence and changed
platforms cannot produce a stale proposal. This covers pre-application validation;
editing a passenger's lines after boarding remains a separate live-test case.

The candidate module still uses only the shared SDK. ABI 7, the single shared
hook, the native Begin/Remove/Add/End operation and component layouts are unchanged.
No additional extender, engine patch or per-mod hook was introduced.

## Bounds and automated validation

A discovery callback examines at most four vehicles, screens four passengers
against one ready vehicle, and performs one full validation. A successful proposal
requires fresh host revalidation. A stale arriving route ends that screening pass.
The worst fixture includes four vehicle checks, four passenger screens, two
four-interval validations, staging and native pre/post reads: **62/64 queries**
(counting observation calls conservatively). Tracking uses separate callbacks.
No query, attempt or home-confirmation bound was increased.

Fixtures cover one to four intervals, direct and partial express substitutions,
branches/reversed order/repeated stations, original endpoint preservation,
platform mismatches, route end/wrap/shortening, inconsistent counts, failure at
every query with preserved output, arbitrary line IDs and stop indices, dynamic
stop discovery, stale line/vehicle recovery, cache replacement and route edits
between proposal and revalidation. Normal and lifecycle-reserved consumer suites
retain home invalidation, removal, identity and continuation coverage.

Shared bridge, copied query/mutation, host batch/failure and both DLL contract
suites also run in the isolated `tools/runtime/bridge-test` Wine prefix. Build
warnings are errors. Automated evidence passes; the first live capture stopped
on waiting-reader errors, with one adjacent journey completed. Multi-stop/express
coverage remains pending. See the 0.8.1 correction below.

## Prepared build and next live trial

Ignored build: `analysis/native-bridge/build-v8a/`. Build command:

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v8a \
  --passenger-limit 5 --observation-limit 18000 \
  --observe-person 86553 --observe-destination 86548
```

The lifecycle-reserved fifth attempt is OFF for this route trial. The optional
observer is compiled for contract testing; load only the mutable DLL. Default
builds still allow one passenger. This remains an explicit development loader,
not an installed/autoload release.

Next restart into a disposable expanded baseline and leave it paused. Verify the
fresh process/save before loading; previous DLLs remain mapped until game exit.
Capture up to five distinct attempts at normal speed without vehicle/line edits.
Check proposal line IDs and original/replacement stop indices, both-world boarding,
planned alighting and separate final outcomes. An adjacent-only capture does not
validate multi-stop or express behavior. Include a passenger with a leg spanning
multiple shared stops and, for express coverage, a substitute skipping an
intermediate stop while retaining both endpoint platforms. A through passenger
whose alighting stop is absent from the substitute must stay on the original line.

No test game was touched during this implementation. Broader platform walking,
local services with extra stops, journey-time comparisons and individual
preferences remain future work. Home-loss handling still lacks a matching live
case despite passing fixtures; removal continuation was proven in 0.7.5.

## Evidence artifacts

All nine final test invocations pass. Results and executable hashes are in ignored
`build-v8a/route-expansion-tests.json`; final binary hashes are recorded beside it.
Prepared `watch_trial.py` and `audit_trial.py` are inactive. After a fresh load,
create `live-run.json` with the actual log name; the audit reports multi-stop and
express boarding coverage separately and honors whether the fifth was reserved.

- `tpf_shared_bridge.dll`: `37be1d7b68e71ed81d3f968c2d106ecc991e0e6018f8542a5c656fcf771aeb18`.
- `tpf_journey_observer.dll`: `88032fd550eb130c5ba118c2089d02de8d0acdbe5cf74e3c4a9385f2786ee859`.
- `route-expansion-tests.json`: `6090d0b0a9da68c1b225d6854934768c6e6f3819d3a82610398b9869aa5c8d6e`.

## Historical live trial startup — 2026-10-09

The user restarted into the verified original baseline and confirmed normal-speed
unpause after loading. Linux PID17502/start624621, Windows PID312; no previous
bridge mappings. Startup returned0 and confirms version0.8.0, cap5/18000,
reserve OFF and route bound4. Log `build-v8a/tpf-bridge-312-6357383.log`;
adjacent `live-run.json` records full metadata. Capture is active; no outcome
or multi-stop/express validation is claimed yet. Do not reload another DLL.

## Live result and correction — 2026-10-09

0.8.0 stopped at eight waiting-reader invalid-state errors. One adjacent S1→S3
switch boarded and completed original destination86033; no multi-stop/express
switch occurred. The capture and subsequent paused read-only inventory are
finished. Unsupported native vehicle sentinel-3 was found in untouched waiting
passengers, including the final discovery candidate inferred from the frozen
cursor. Version0.8.1 preserves known-2/-3 values for discovery while excluding
both from switching; all other guards remain. Build-v8b subsequently completed
18,000 callbacks with three adjacent boardings, two original arrivals and one
onward journey unfinished at capture end. Zero query errors, no special waiting
samples, no multi-stop/express boarding. All watchers ended; the wrapper forwards
only. The next implementation separates attempt capacity from journey diagnostics
and adds discovery metrics; no extra diagnostic save reload is needed now.
See [evidence, fix and coverage limits](waiting-sentinels.md).
