# Boarding-window selection — 0.12.0

## Problem and scope

Version 0.11.0 confirmed an S1→S2 return boarding at Wimborne, but only one
switch on that departure despite more reported waiters. Its exact live
bottleneck remains unproven. Code inspection identified a selection problem:
all trains at a station advance the same passenger cursor, and incompatible
directions can consume the one full validation allowed per discovery callback.
An offline mixed-direction fixture reproduces poor coverage.

## Implementation

Each remembered vehicle now has independent general and preferred passenger
search positions. Station hints retain the source line and boarding/alighting
indices already read during discovery. After a successful proposal, three of
four selection opportunities prioritize other hints for that same planned leg.
The fourth explores other legs; a missing preferred hint falls back immediately.
General exploration has its own cursor, so preferred searches do not skip its
passengers. Vehicle line/stop changes and cache replacement reset its search.

Observed ready vehicles receive three of four first vehicle picks, with global
exploration on the fourth and remaining checks. Every proposed passenger still
receives full fresh validation and deferred engine revalidation. Hints authorize
no mutation and add no SDK queries. ABI 8, native operation, four-hop policy,
platform/capacity guards and one-unconfirmed-boarding rule are unchanged.

New counters distinguish preferred picks, tracking callbacks and callbacks spent
waiting for boarding confirmation. No train ETA comparison or passenger fare/
comfort preference model is introduced. Search ranking does not guarantee that
an entire queue will board during a short stop.

## Offline validation

All fourteen native invocations pass; executable hashes match the report.
The existing worst-case query budget remains 63/64, and the expanded discovery
fixture peaks at 44/64. It retains sparse-world discovery, stale/deleted vehicles,
changed passenger hints, deferred route/platform edits and lifecycle coverage.

The same fixture compares the 0.11.0 consumer (`b888541`) against 0.12.0:

| Scenario | 0.11.0 | 0.12.0 |
| --- | --- | --- |
| 80 waiters, 20 compatible, 120-callback window, 16 phases | 7–8 switched | 12–13 switched |
| Both compatible source lines served in that window | 0/16 phases | 16/16 phases |
| Two simultaneous opposite-direction arrivals, 8 phases | At least one train unserved in some cases | At least 10 switches per train in every case |

These are synthetic selection results, not in-game speed or waiting-time
measurements. `TFX_DISCOVERY_BASELINE` disables only the new throughput assertions
when measuring the old consumer. Other fixture checks remain active. Regression
thresholds require at least twelve switches and both source lines in every mixed
phase, plus at least four per train in every simultaneous-arrival phase.

## Prepared trial

Build directory: ignored `analysis/native-bridge/build-v12a`. Limits remain
32 distinct attempts, four concurrent journey samples and 18,000 callbacks.
Package: `TPF3Interlining-0.12.0-trial.zip` in that directory; its manifest
records source and file hashes. It contains the mutable DLL, loader, test and
comparison reports, and instructions. The test-only observer is excluded.
Version 0.12.0 has **not** been loaded. Continue the existing 0.11.0 capture;
do not inject this DLL into its process. The next integrated trial requires one
fresh restart, disposable baseline, process/save/hash verification and paused
loading using the [native guide](../native/README.md).

Compare switches per loading window at Wimborne in both directions, source-line
coverage, new selection counters, boarding confirmations and sampled original
destinations. No persistent installation or stable release is claimed.

- DLL SHA256: `29a9dfca0052c1890dd34d7d9dcd4164d6f2873d5522696e3220c63df2e75a3b`.
- Test report SHA256: `483474668d0c175eb4db0b51a027aecc66643e2efdd0eef0a8031a7a6f3dc527`.
- Report timestamp: `2026-10-11T10:58:38.563903Z`.
- Raw old-consumer comparison: `build-v12a/baseline-comparison.json`.

## Early live evidence — S1 at Wimborne toward Faringdon

The prepared build was loaded after fresh process/save/hash verification on
2026-10-11; the user confirmed normal-speed unpause. During S1 vehicle98398's
return stop, the user reported16 S1,5 S3 and3 S2 waiters, then observed the S2
queue clear while some S3 passengers remained on the shared platform.

The log confirms three S2→S1 changes preserving Wimborne→Faringdon (5→0):
75645 boarded both worlds at1240,108642 at1245,107729 at1249. All three were
unsampled; final destinations remain unknown. No S3→S1 switch was logged for
this stop. At1258 the same train still had15 free places; the next logged
validation rejected112636's opposite-direction S2 leg1→2 as alighting mismatch.

This supports the user's report and identifies incomplete selection coverage,
not a platform-access rejection or proven ineligibility of the five S3 waiters.
Their individual records were not captured. Successful-leg ranking includes
source line, so it can favor remaining S2 hints before general exploration;
the exact cause of each missed S3 opportunity remains unproven. Continue the
trial; do not weaken route guards or claim the entire queue was evaluated.
Ignored `build-v12a/wimborne-s1-faringdon-01.json` and `.log` preserve the event.

### Repeated S1 return opportunity and attempt cap

The user reported another arrival, initially6 S3/3 S1/1 S2 waiters, then12 S3
waiters remaining with none boarding S1. For vehicle98594, the log confirms
109362 and119096 switching S2→S1 and boarding both worlds at2342/2349. Spare
places at validation were36/33. No S3 switch was recorded for this stop. The
initial S2 count differs from the two observed boardings; queue membership may
have changed during the observation. Preserve both observations without assuming
individual eligibility or exhaustive screening of the twelve S3 passengers.

These were attempts25/26, before the cap; the cap did not cause this missed S3
opportunity. Same-source selection versus broader compatible-leg coverage is
the next implementation focus. Evidence:`wimborne-s1-faringdon-02.json/.log`.

At11:27Z all32 attempts had confirmed boarding, last at2576. Seven lifetime
samples: two original arrivals (72701/109709), one home invalidation (75515),
four still travelling (100621/111528/72084/85913);25 other boardings unsampled.
No errors or stop/summary marker. The loaded trial now only tracks remaining
samples; later departures cannot produce new switches. Evidence is preserved
in ignored`attempt-cap-checkpoint.json/.log`. This remains a partial trial.
