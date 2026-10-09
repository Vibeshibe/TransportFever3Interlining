# Waiting vehicle sentinels — 0.8.1

## 0.8.0 live result

The first demand-driven route trial stopped at its existing eight-query-error
limit. Passenger 40365 switched S1→S3/97912 at callback 954, boarded in both worlds
at 981, alighted at planned stop 2 at 1952/1953, transferred to S2/98301 and reached
original destination 86033 at 4704/4705. The remaining four attempts were unused.
The completed leg was adjacent; **multi-stop and express coverage remain false**.

All eight errors had status 14 (`TFX_INVALID_STATE`) while reading other waiting
passengers: five in discovery and three in candidate screening. Final summary:
`reason=query_errors`, staged 1, accepted 1, query_errors 8. `switching_blocked=0`
is a separate lifecycle latch; the pilot is stopped and the wrapper explicitly
forwards only, permitting no further changes. The stopped-trial log audit passes
its preservation/boarding/arrival invariants; it does not label the trial a success.

Watcher 44054 exited0 at 2026-10-09T09:43:55.329225Z after 1098.2 seconds. User paused
the game for read-only inspection. No restart, second DLL, debugger or engine call
was used for that inspection; the reader is closed and no watcher remains.

## Concrete evidence

Read-only `/proc/PID/mem` access copied the stopped consumer's 32 cached IDs and
inventoried their components in both worlds. Repeated reads agreed and modification
windows were closed. These are observational snapshots, not atomic transactions.
Three untouched waiting passengers—72593,75640,86317—had vehicle field **-3** on
line 78942, while SimPerson's first word was 0 and line/stop fields were nonnegative.
Our waiting reader rejected any vehicle value below-1.

Frozen discovery_cursor 3209 and person_count 32 identify the final attempted slot
as `31 - ((3209 - 1) % 32) = 23`, containing 86317. The last logged error was
`discovery_waiting`; both worlds' later paused records retain its -3 value.
This identifies the last candidate and a concrete failing guard. Earlier errors
lack passenger IDs, so their individual causes are not all attributed.

Existing [boarding analysis](native-boarding-investigation.md) already identified
separate native branches for -1, -2 and -3. The exact meaning of-2/-3 is not established;
neither is assumed to mean ordinary unassigned waiting. Only 40365 was mutated
in this run, and it completed its original journey.

## Implementation

The copied waiting-leg query now preserves the known -2/-3 values. All other
negative values below-1 still return invalid state. No negative value is normalized
to-1. Destination, waiting age/place, component structure and read guards remain.
SDK layouts, ABI7 and the native mutation operation are unchanged.

The candidate validator labels these passengers `unsupported_waiting_state`.
Discovery can learn their line, but candidate screening skips them. The host and
native adapter independently continue requiring vehicle exactly -1, even for a
fabricated request. Tracked journey queries retain their stricter existing guard;
this change addresses discovery only, without interpreting sentinel transitions
on a passenger already selected for switching.

Up to eight `pilot_waiting_excluded` messages record sampled passenger/sentinel
pairs. `special_waiting_samples` counts observations, not unique passengers.
Waiting-read errors now include person and callback sequence. No lifecycle,
route, query-budget, eight-error or attempt limit was relaxed.

## Verification and next trial

Regression cases preserve exact-2/-3 waiting snapshots, exclude both in candidate
policy and actual discovery, and demonstrate another eligible passenger can still
be staged. Fabricated host/backend requests cannot invoke engine operations;
unknown -4 and existing malformed data remain errors. Multi-stop, route-edit,
lifecycle, host, mutation/query and DLL-contract regressions are retained.

Prepared version 0.8.1 under ignored `analysis/native-bridge/build-v8b/`: cap 5,
18,000 callbacks, route bound 4, lifecycle reserve OFF. The optional observer targets
40365/destination 86033 for contract testing only. The fresh live capture below is now running; outcomes remain pending.
The current 0.8.0 DLL remains mapped until exit, so a revised trial needs a fresh
restart into a disposable `TPF3Interlining_before_multi_passenger_01` baseline.
Keep the same route scope and validate known-sentinel exclusions separately from
multi-stop boarding. An adjacent-only result cannot validate the broader policy.

## Evidence locations

Historical Linux PID 17502/start 624621, Windows PID 312, thread 592; worlds
`0x3102c650`/`0x3102b990`. Never reuse after restart/reload.
Files under ignored `analysis/native-bridge/build-v8a/`:

- `tpf-bridge-312-6357383.log`: `921145437a82b478b06277300dd49337d2e47802629c8e68a48f3ac7900417a9`.
- `discovery-stop-inventory.json`: `4d1411b98b52ce2d44a4d98ef60c29c46c4d5382722c30c1fa09381746c44d56`.
- `live-audit.json`: `a3510cbec8c9431f50cc6e9d414c57f2f4a839769b02f3ea309f6de119dc884a`.
- `live-run.json` records completed capture metadata; `inspect_discovery.py` records
  the pinned read-only inspection. No game/save files were written by our tools.

All nine final test invocations pass; peak consumer fixture remains 62/64 queries.
Final evidence under ignored `build-v8b/`:

- `tpf_shared_bridge.dll`: `c3827f24e5e25bcc89d506a55ccce5a58181130cd4675fd3e9343a459ea6103e`.
- `tpf_journey_observer.dll`: `687ff5b9b7619fc088fd90e0d756d8381615c6026efeeb61717cd180fe0068b2`.
- `sentinel-tests.json`: `cd6ed0449197febd380be32afffde40243609ef6b59bbd186fbea8ff4d9362ef`.

## Corrected trial startup — 2026-10-09

Fresh Linux30963/start797666, Windows304; original baseline hash unchanged and
no prior bridge mappings. Loaded build-v8b0.8.1, tfx_start0; user confirmed
normal-speed unpause. Log `tpf-bridge-304-8080293.log`, with adjacent live-run.json.
Cap5/18000, route4, reserve OFF. Capture is active; no live outcome is claimed yet.
