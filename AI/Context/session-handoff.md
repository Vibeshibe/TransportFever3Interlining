# Session handoff — 2026-10-09

## Latest — alternative-platform research; 0.9.1 still running

User requested research while the existing trial runs. Completed read-only
Ghidra/assembly investigation and official-manual lookup; see
[alternative-platform behavior](../../docs/alternative-platform-behavior.md).
Native assignment attempts a short vehicle-entry walk only when an entry is
within 5 world units. Empty approach falls through to direct onboard transition
(boolean return verified in assembly despite incorrect Ghidra void signature).
Walking completion also reaches onboard through NoteWalkPersonsArrived. This
does NOT prove that all alternative-platform passengers teleport: upstream
relocation/rerouting, full walking cost and ordinary departure recovery remain
unresolved. Keep current platform guards. Focused same-line native alternative
test is later work; no restart, debugger or DLL change during this capture.
All targeted Ghidra exports finished; ignored evidence manifest and addresses
are recorded in the guide. Documentation only; native source/build unchanged.
User subsequently reported apparent teleport boarding on an excessively long
platform and suggested alternatives behave similarly. Recorded as visual user
evidence consistent with the fallback, not a timed trace or confirmed alternative
case. It strengthens the reuse hypothesis without proving walking-time handling.
Latest clarification: user is certain of the behavior in Transport Fever 2.
Keep TF2 precedent separate from TF3 static findings; do not label it a confirmed
TF3 alternative-platform observation.

Latest live checkpoint6600:7 staged/applied/boarded,6 sampled original arrivals,
1 active sample,peak4,0 pending/unsampled boardings, no logged errors.
Discovery:123 indexed picks,920 fallbacks,1 rejected stale-station hint,
802 station mismatches/1043 screens,71 validations. >5 switches now demonstrated;
unsampled boardings and multi-stop/express live coverage remain pending. Seven
boarded IDs:115753,93862,110796,99871,111528,125652,108250.125652 remains tracked.
No summary yet. Existing log/watch details below remain applicable; no new DLL.
This checkpoint supersedes older interim claims of no >5-switch evidence.

## Current running checkpoint — resume by reading the log

User confirmed unpause; capture continues automatically, not complete. Watcher
session54222 remains active (one-hour wall limit); DLL self-limits at18000
callbacks. Avoid spending usage on frequent empty polling. Read live-run.json
and the latest log on next turn before acting. No restart or second DLL.
At1800:4 staged/applied/boarded,4 active samples,peak4; index picks37,
hint mismatches0,fallback212,station mismatches195/screens249,26 validations.
115753 reached original75525 at2145/2148, freeing a sample. Other confirmed
boardings:93862,110796,99871. No query errors, no unsampled boarding or >5
switches yet. All replacement legs observed so far are adjacent. These remain
interim findings; do not run completion-only audit or claim capture success yet.
Later checkpoint:93862 reached original72810 at2304/2305 and110796 reached
original72712 at2429/2432:three completed journeys, fourth99871 still tracked.
At2400:54 indexed picks,1 stale-station mismatch rejected,282 fallback picks,
256 total station mismatches/336 screens,28 validations. Zero query errors.
An inactive audit_trial.py is copied into build-v9b; run only once pilot_summary
and the hook-forwarding completion marker exist, then inspect any assertion
failure rather than relaxing it. Live-run.json stores exact log/process identity.

## Latest — 0.9.1 running after confirmed unpause

User confirmed normal-speed unpause. Watcher54222 active; thread752,
worlds1/2 0x5f512630 / 0x5f511970 (only for this process). At600: three successful
changes and both-world boardings (115753,93862,110796), three active samples,
no arrivals yet. Index picks12, hint station mismatches0; fallback87,
station mismatches83, screens99. Prior0.9.0 at600 had one boarding and128 station
mismatches/167 screens; encouraging descriptive comparison, not a controlled
performance result. No query errors. Continue bounded capture; no new DLL.

User confirmed restart/readiness. Verified fresh Linux107579/start1780715,
Windows312, baseline hash ce2e02d935da…, pinned executable and prepared DLL hashes,
and no previous bridge mappings. Actual stdout load before_multi_passenger_01
at12:40:07Z; ready12:40:54Z. Loaded build-v9b successfully at12:41:51Z,
tfx_start0. Startup confirms0.9.1, station_index,32 attempts,4 samples,18000
callbacks,600 boarding grace and reserve OFF. Startup was observed while paused; subsequent unpause/checkpoints are recorded above.
Log: analysis/native-bridge/build-v9b/tpf-bridge-312-18170090.log
(the exact filename is recorded in live-run.json). preflight-process.json and
loader-result.json preserve verification. Watcher session54222 is running. User has been asked to unpause at
normal speed without line/vehicle edits. Normal-speed unpause is now confirmed; current checkpoint above supersedes this startup note. Compare indexed/fallback selections and station mismatch
work; monitor boarding, samples, errors and fixed cap. Do not load another DLL.

## Preparation — 0.9.1 station-focused discovery

User wants implementation that addresses sparse matches with80+ waiting, without
spending further usage extending the old run. The0.9.0 capture is stopped and its
watcher ended. No game interaction or replacement load during this implementation.

Version0.9.1 learns one waiting station directly from each scanned page, rotating
positions on completed sweeps. Tests exposed omissions when learning through the
churning32-person global cache; direct page learning resolves the fixture case.
New station_cache.h retains16 groups ×32 person/line hints, with round-robin
selection/eviction and no engine pointers. First three candidate screens prefer
the arriving station and another line, fourth uses global exploration if reached.
Every proposal retains fresh copied reads and deferred validation. Known stale,
assigned and selected hints are discarded; moved hints migrate. Existing32-attempt,
4-sample,1-pending,18000-callback limits, ABI7 and mutation policy are unchanged.

Prepared ignored build-v9b, never loaded. Mutable DLL SHA:
a21d9fa8548c43c91be01d4210aff43de7295c872ecc233e3d40c804fd32cacb;
optional contract-test observer SHA:
a9a6070094cc2d7c3e91d61dd53f42e12c5c8fb316b112c37accf9c745d1d407.
All13 final native invocations pass, including all80 candidate positions in
normal/reserved/sampled fixtures and legacy32-attempt coverage. Max63/64 queries.
capacity-tests.json SHA:dfa2b4064fb4b0a2d2bfe5b0fb5f64312508ad5af1bbb2c18a9a8e63b8034cb6.
All test sessions ended. Read docs/station-discovery.md for behavior, bounds, tests and next trial.
A station cache is a rolling32-person window, not a full80-person queue. Live
throughput improvement, >5 switches and unsampled boardings remain unproven.

Next: ask for actual restart/readiness into disposable
TPF3Interlining_before_multi_passenger_01, paused. Old0.9.0 remains mapped until
exit. Verify fresh PID/start, actual loaded save, pinned executable, new DLL hash
and no previous bridge before loading build-v9b. Never reuse old addresses or
load beside the stopped DLL. No extra diagnostic reload planned. Compare indexed
picks, fallbacks and station mismatches, then confirm boardings/sample outcomes.
Keep the bounded capture rather than extending it to fill the cap.

## Historical — 0.9.0 stopped; station-discovery work requested

User reports80+ waiting and requests avoiding usage on repeated inefficient
search. Stopped current bridge through existing tfx_stop: returned0 at
2026-10-09T12:21:00.533606Z. Watcher28281/owned Linux77511 terminated intentionally
(exit143). All observers ended; no further capture needed. DLL stays mapped until
game exit. No replacement is loaded yet. Implement/test station-focused search
before requesting a new game process; user is willing to restart for improvements.

All three switched passengers reached original destinations:122261 at2420/2421,
127526 at6776/6777,120415 at8645/8648. Latest metrics9000:three samples,peak1,
active0,zero errors. Last candidate log9419; stop does not produce a pilot_summary,
so exact stop callback/final aggregate values are not invented. Three adjacent
legs, including S1 stops4→5 toS3 stops5→6; no >5 or unsampled-boarding coverage.
Partial stopped audit validates all changes, boardings and arrivals; separate
from prepared automatic-completion audit. Log SHA76c3062be72c7d43a6f7061c63ab3c28298208c70d02f36f97e9fbc30b8cd34e;
stopped-audit.json SHAb258775a776d98cb0f6183f86f1f0700116ac1a592fad281aee8efdbd8497e65.
stop-result.json/live-run.json record stop acknowledgement. No source changes yet
for the next discovery version at this checkpoint.

## Historical — 0.9.0 running checkpoints

User confirmed restart/readiness. Verified fresh Linux68800/start1296244,
Windows308, pinned game hash and no previous bridge mappings. Stdout confirms
original before_multi_passenger baseline load11:15:52Z, ready11:46:44Z; baseline
hash remainsce2e02d935da…,80,431,012bytes. Prepared artifact hashes all match.
Loaded build-v9a0.9.0 successfully, tfx_start0. Startup log confirms32 attempts,
18000 callbacks,four samples,600 boarding grace,reserve OFF,route4.
Log `tpf-bridge-308-14937416.log`; ignored live-run.json and loader-result.json
record fresh identity and startup. User confirmed normal-speed unpause.
Watcher28281 active; worlds1/2 `0x369cde50`/`0x369ceb10`, thread616.
Checkpoint6300:122261 S3→S1/97745 staged418, boarded427, original72934 arrival
2420/2421. Tracker released; next600-callback report confirmed active0.127526
then switched S1 stops4→5 toS3/97912 stops5→6 at5097, boarded5114, alighted5909/
5912 and boarded native79462/123307 stops0→1 at6180/6181. Original98211 pending.
Two samples cumulatively, peak1 tracker: reuse observed, no unsampled release or
>5 switches yet. Both replacement legs adjacent; no multi-stop/express coverage.
Zero query errors. User reports50+ people waiting and asks about sparse matches.
Counters support discovery inefficiency:1172/1489 screening attempts at4800
were station mismatches. These are repeated checks, not unique people. Prioritize
station-focused discovery after this bounded trial, retaining all eligibility
guards. Do not infer how many opportunities were missed or blame RNG alone.
Do not reinject or reuse any historical addresses below. Continue monitoring
boarding, sample selection/release, totals/discovery metrics and error guards.

## Preparation — 0.9.0 sampled capacity

User authorized implementation after the completed0.8.1 capture. Implemented a
compact session ledger separate from reusable full journey trackers. Cap1–32,
default1; ≤5 defaults full tracking, larger trials max4 samples plus1 pending.
All switches retain fresh route/state revalidation and both-world boarding.
600-callback boarding/result timeout; no refund or retries. Released IDs remain
excluded. Unsampled post-boarding journeys are no longer read and never add arrivals.
Sample selection is first available slot at boarding, not a representative sample.
Existing32-callback home guard, error/lifecycle stops, ABI7/hook/native operation
unchanged. New aggregate counters every600 callbacks and at shutdown; discovery
uses all available callbacks when no trackers remain. No larger search windows or
platform policy changes; live performance is unmeasured.

All13 final native invocations PASS.32-person actual consumer test:4 retained
samples,28 released boardings, peak5 trackers, max62/64 queries. Legacy normal/
reserve lifecycle tests retained; host capacity/failure/history, copied query/
mutation, candidate and DLL contracts pass. Nine C/CMake option combinations and
six invalid CLI combinations checked. Corrected a test-population pagination bug
during development. All test sessions ended; no live reader/watcher/game action.

Prepared ignored `analysis/native-bridge/build-v9a`:32 attempts/18000 callbacks,
4 samples, reserve OFF. Optional observer75561/destination79212 is contract-test
only. Mutable226347e0a0befe95837f3c4dfcf0831e87fc2c5ecc263d96fef1760320dbf14c;
observera8293373faf78eebc4771dbd8b2241af4b82316255612500e14f446eb8694ece;
tests928472786c99aecdc669826553a543f43055b0d620af9cce86614658622544b9;
configd08f5a80efe248221c8f988226fa1321995446a8578e336af5469dc41e7a94b9.
`capacity-tests.json`, `configuration-tests.json`, `capacity-hashes.json` preserve
evidence; `run_tests.py` is the isolated Wine runner. Inactive watcher and sampled
audit are prepared and syntax-checked, not run against a live0.9 log yet. They need
fresh `live-run.json` after a verified load. No0.9 capture or outcome exists.

Next integrated trial requires actual readiness after restart into disposable
`TPF3Interlining_before_multi_passenger_01`, paused. Verify PID/start, actual loaded
save, executable/DLL hash and absent old bridge. Never reuse historical addresses
below or inject beside0.8.1. User has not confirmed a restart for0.9 yet. No need
for extra diagnostic reloads before this integrated trial. Monitor >5 replacements,
unsampled boarding, sample reuse and metrics if demand permits; report missing
coverage honestly instead of extending caps. Multistop/express, special waiting
states, whole-trip timing and preferences remain separate gaps.
[Implementation and full evidence](../../docs/passenger-capacity.md).

## Previous — 0.8.1 corrected route trial complete

User restarted into the verified unchanged original baseline, then confirmed
normal-speed unpause after loading. Fresh Linux30963/start797666, Windows304;
no prior bridge mappings. Stdout load09:52:21Z, ready09:53:10Z. Loaded build-v8b
0.8.1 mutable DLL, tfx_start0; cap5/18000, route4, reserve OFF. Log
`tpf-bridge-304-8080293.log`; ignored live-run.json records fresh metadata.
Worlds1/2 `0x304d3520`/`0x304d2860`, thread592. These are historical run identities;
revalidate before any future access. Capture stopped normally at18000:
three switches/boardings, two original arrivals, zero query errors, no anomalies.
122261 S3→S1/97745 staged418, boarded427, original72934 arrival2436/2437.
128483 S1→S3/97959 staged10210, boarded10212, original72113 arrival12245/12248.
75561 S3→S1/97745 staged13242, boarded13244, planned alight14077/14080;
then native S3/97912 stops2→5 boarding15988/15989. Original79212 arrival unobserved
when capture ended. No arrival/failure inferred. All replacements adjacent.
No special waiting samples, multi-stop or express boarding. Final counters:
963 ready samples,92 validations,3 eligible,65 boarding mismatches,8 alighting
mismatches,14 unsupported legs,2 divergence. These are checks, not unique people.
Watcher59201 hit its3600-second wall limit; final watcher33900 confirmed forwards-
only native shutdown and exited0 at10:54:36.102670Z. **All watchers/readers ended.**
No further game time or replay needed. Do not reinject into this process.
Log audit passes; log SHA654644bea15ac323667065a66a4a8363b9e8fb5769731b8c2d797f7c2991988a;
audit SHAfcda90bf4de5c219c83ea3ad7faa1619e7ccb94f030671e1119db10be27bb8b4.
[Full result and limitations](../../docs/waiting-sentinels.md).
Prepared a [capacity implementation plan](../../docs/passenger-capacity-plan.md)
while monitoring; proposed32 attempts/one pending/four diagnostic samples is NOT
implemented or loaded. Next progress implementation and offline tests before
another integrated live test; do not ask for a separate diagnostic reload now.
User asked about sparse examples: explained traffic timing, expanded demand
sampling and platform exclusions. Raising the cap alone does not fix discovery.
All current limits and source binaries remain unchanged.
Historical process/world addresses below must not be reused.

## Previous — 0.8.0 stopped; 0.8.1 sentinel correction prepared

0.8.0 build-v8a trial is FINISHED: reason=query_errors, staged1, query_errors8,
switching_blocked0 (separate latch; pilot_done and forwards-only wrapper stop writes).
40365 switched S1→S3/97912 at954, both-world boarded981, alighted1952/1953,
then S2/98301 and original86033 arrival4704/4705. No multi-stop or express coverage.
Watcher44054 exited0 at2026-10-09T09:43:55.329225Z. User paused and left game open.
**All watchers/readers/tests ended. No further running time needed for diagnosis.**

Paused read-only inspection copied stopped consumer caches plus repeated records
in both worlds. Untouched72593/75640/86317 have waiting.vehicle=-3 on78942,
SimPerson state0, valid positive line/stops. Frozen discovery_cursor3209/count32
implies finalslot23/person86317; old query rejects vehicle<-1. Earlier native
analysis documents distinct-2/-3 branches; exact meanings remain unestablished.
Only final candidate is inferred; earlier errors lack person IDs. No debugger,
second DLL, engine call or memory write was used. Original baseline ce2e02d935da…
remains80,431,012bytes. No save changed by our tools.

Prepared0.8.1 `analysis/native-bridge/build-v8b`, cap5/18000, route4, reserve OFF;
NOT loaded. Copied waiting query preserves known-2/-3 without normalizing them;
policy/discovery skip switching these states. Host and native adapter still require
vehicle exactly-1. Unknown negatives and tracked journey-state guards unchanged.
Bounded exclusion logs/counts; waiting errors now include person and sequence.
All9 tests PASS, peak62/64, including fabricated sentinel requests with zero native
operations. Native ABI/hook/mutation/lifecycle/route bounds unchanged.
Mutable DLL c3827f24e5e25bcc89d506a55ccce5a58181130cd4675fd3e9343a459ea6103e.
Observer 687ff5b9b7619fc088fd90e0d756d8381615c6026efeeb61717cd180fe0068b2.
Tests cd6ed0449197febd380be32afffde40243609ef6b59bbd186fbea8ff4d9362ef.
Optional observer40365/destination86033 is for contract tests, not concurrent use.

Old process Linux17502/start624621, Windows312, thread592; worlds0x3102c650 /
0x3102b990. Old0.8.0 stays mapped until game exit; DO NOT reinject. Next requires
fresh restart into disposable before_multi_passenger baseline paused, fresh process
and save verification. Await readiness before loading build-v8b. No increased caps.
Prepared inactive watcher/audit under build-v8b need new live-run.json after load.
Check pilot_waiting_excluded and zero reader failures alongside route coverage;
adjacent-only success does not validate multi-stop or express substitution.

Evidence under build-v8a: log6357383 SHA256
921145437a82b478b06277300dd49337d2e47802629c8e68a48f3ac7900417a9;
discovery-stop-inventory.json4d1411b98b52ce2d44a4d98ef60c29c46c4d5382722c30c1fa09381746c44d56;
live-audit.json a3510cbec8c9431f50cc6e9d414c57f2f4a839769b02f3ea309f6de119dc884a.
Audit preserves successful adjacent evidence but trial_completed_successfully=false.
[Full result, correction and next scope](../../docs/waiting-sentinels.md).

## Historical — 0.8.0 route trial startup

Fresh process and original baseline verified; user confirmed normal-speed unpause.
Linux17502/start624621, Windows312; no prior bridge mappings. Stdout baseline
load09:23:31Z, ready09:24:17Z. Loaded build-v8a mutable DLL, tfx_start0;
cap5/18000, reserve OFF, route bound4. Log `tpf-bridge-312-6357383.log`;
ignored live-run.json records hashes/process metadata. Watcher44054 active; worlds1/2 `0x3102c650`/`0x3102b990`, thread592.
No live result yet. Do not reinject. Monitor multi-stop/express coverage, boardings and final outcomes.
All historical addresses below expire on restart/reload.

## Preparation — 0.8.0 route expansion

User authorized implementation after the completed 0.7.5 trial. Implemented
bounded demand-line discovery (no baseline IDs) and multi-stop candidate matching:
original legs span1–4 forward intervals; replacement follows an ordered subset
through the ORIGINAL alighting stop, allowing express skips. Extra stops/detours,
repeated original station groups, wrap and alternate endpoint platforms reject.
S1↔S3/stops1→2 mutation gate is removed. Exact copied leg records are compared at
deferred revalidation, rejecting even matching edits to both routes. ABI7, shared
hook, engine mutation functions, cap and lifecycle policy are unchanged.

Discovery:32 people/16 lines/16 vehicles with rolling replacement and reusable
absent entries; one demand leg plus one line-stop sample per discovery callback.
Independent stop cursors; no complete network enumeration. Services without
observed waiting demand may be missed. Screens4 vehicles/4 people, one full
validator; absent arriving route ends screening. Worst fixture62/64 queries.
All9 invocations PASS: candidate, pilot, reserve, bridge, query/mutation,
host batch/failure and both DLL contracts. Final tests exited; no watchers active.

Prepared ignored `analysis/native-bridge/build-v8a/`, version0.8.0, cap5/18000,
reserve OFF. Optional observer86553/destination86548 is built for contract tests,
not simultaneous loading. **Mutable DLL NOT loaded; no game/save touched.**
Mutable SHA256 37be1d7b68e71ed81d3f968c2d106ecc991e0e6018f8542a5c656fcf771aeb18.
Test evidence SHA256 6090d0b0a9da68c1b225d6854934768c6e6f3819d3a82610398b9869aa5c8d6e.
`route-expansion-hashes.json` has both DLL hashes; `route-expansion-tests.json`
has all final results/executable hashes. Existing v7f artifacts preserved.

Next fresh restart into disposable before_multi_passenger expanded baseline,
paused; verify fresh PID/start/save before loading build-v8a. Do not reuse the
historical process/worlds below. Await fresh-game readiness before loading.
Prepared inactive watch_trial.py/audit_trial.py need a fresh live-run.json.
Audit now reports multi_stop_boarding_observed / express_boarding_observed;
an adjacent-only run does NOT validate the expansion. Five attempts, no reserved
slot for this trial. Require a multi-stop planned shared leg and an express
substitute for that separate case, same endpoint platforms; observe boarding,
planned alighting and final outcomes. No automatic bounds increase or reload.

See [route expansion](../../docs/route-expansion.md) for examples, limits, build,
artifacts and test procedure. Existing home_invalidated remains fixture-only;
0.7.5 proved removal continuation. Platform walking, extra-stop local substitutes,
full-trip estimates/preferences, autoload and a stable release remain open.

## Previous — 0.7.5 completed; continuation after removal proven

**Five switches/boardings, four original arrivals, one confirmed removal.**
At13695 stopped automatically with all_outcomes_observed, query_errors0 and
switching_blocked0. Wrapper forwards only; no further changes permitted.
Watcher73040 exited0 at2026-10-08T18:53:18.205804Z. All watchers/readers ended.
User told game may be paused/closed; no further runtime, checkpoint or restart
needed for this completed test. Do not reinject while the DLL remains mapped.

120842 S3→S1/97745 stage512 boarded514, removed7189/7190, arrival0.
122261 same train stage518 boarded520, original72934 arrival2160/2161.
122790 S1→S3/97914 stage2011 boarded2013, original86009 arrival5782/5783.
127558 S3→S1/98398 stage2986 boarded2999, original120856 arrival5238/5239.
86553 S1→S3/97914 stage9214 boarded9247, original86548 arrival13694/13695.
Discovery resumed after removal confirmation7190 with one remaining attempt;
86553 proves a later distinct switch/boarding and completed onward journey.

Log audit PASSES, continuation_proven true. **No home_pending/home_invalidated
event occurred**; home_invalidation_continuation_proven and home_clear_continuation_proven
remain false. Home-specific handling is fixture-tested only. Do not attribute
120842's exact removal cause or claim five arrivals/time savings/broad safety.
No code/bounds changed during capture. Next proceed with bounded route-eligibility
implementation beyond save-specific S1↔S3 stops1→2, retaining existing planned
alighting/platform/capacity/lifecycle guards. User wants implementation next;
do not automatically request another replay of this completed removal test.

Loaded build-v7f0.7.5 source5a5b93e; startup10b5009/interim9e847ec pushed.
DLL SHA25634ad73a8d3aaad513899327ebeaafe22167a47f3ef5d0c99668044f552685b60.
Log `tpf-bridge-304-23489567.log` SHA256
50f6f792028fe0ef4b4e67d033c9046067a0bff9a50299ad8e4beaf315915c4d.
Audit SHA2561957a0824c518bd4d192aeaed5a4d387005cdb0cded81ca0166dfa6b2cc58c63.
Ignored build-v7f/live-run.json has full completed metadata. Original baseline
before_multi_passenger remains ce2e02d935da… /80,431,012bytes; no save overwritten.
Historical Linux102810/start2320640, Windows304, thread592; worlds0x2e7c3ca0 /
0x2e7c4960 expire on restart/reload. Fresh process/save verification always required.
Full result/next scope: [home invalidation](../../docs/home-invalidation.md).

## Previous — 0.7.4 timed out; 0.7.5 prepared

0.7.4 build-v7e trial stopped at2013 with home_clear_timeout, zero query errors,
staged3, switching_blocked1.120770 home87062 cleared to-1 in worlds1/2 at1981/1982;
target72649/assigned slot retained. Still present at32-callback deadline. All
three boarded (120842/122261 S3→S1/97745 at425/429;120770 S1→S3/97914 at1477),
no original arrivals observed, no fourth/fifth attempt or continuation proof.

Watcher48734 exited0 at17:50:14.650952Z. Read-only follow-up57459 started17:51:19Z:
120770 still onboard in BOTH worlds with[-1,-1,72649], target72649. It became
EcsOctreeNode-only at17:51:28.596278Z/17:51:28.800613Z. Follow-up ended exit0,
165 accepted world samples/12 rejections; no alighting/exact removal call captured.
**All watchers/readers/tests ended.** User told game no longer needs to run.
Current0.7.4 forwards only, DLL remains mapped until process exit. Do not reinject.
Last Linux93030/start2190548 / Windows304; world1 0x33227050/world2 0x33227d10,
thread624; all expire on restart/reload. Evidence live-run.json/live-audit.json,
log22022778 and home-timeout-followup.jsonl under ignored build-v7e.

Prepared0.7.5 build-v7f (commit5a5b93e pushed): both-world matching home loss closes as home_invalidated
without arrival/removal claims; affected ID and attempt remain consumed. New
proposals can resume once ALL pending cases resolve and no permanent error block.
One-world evidence keeps32-callback deadline. Restored/reassigned homes, mixed
arrival/invalidation and reappearance still stop. If any removal was seen, require
both-world removal instead; no mixed-state home classification. No ABI/native
write/route/cap changes. Normal/reserved consumer, host/failure and both contract
suites PASS; peak40/64. **NOT loaded, continuation still requires live validation.**

Mutable DLL SHA25634ad73a8d3aaad513899327ebeaafe22167a47f3ef5d0c99668044f552685b60.
[Home invalidation guide](../../docs/home-invalidation.md) records full evidence,
all artifact hashes, policy and next trial. Next restart into before_multi_passenger
baseline paused, load ONLY build-v7f, and prove confirmed home_invalidated followed
by an actual later different-person switch/boarding. **Fresh restart into paused
baseline requested; awaiting user readiness.** Do not inject until old process exits.
Prepared watch_trial.py/audit_trial.py under build-v7f read live-run.json. The
audit accepts home_invalidated only with cleared mask3/removal0 and requires an
actual later boarding for home_invalidation_continuation_proven. Separate original
arrivals, removal and home/destination invalidation.

## Previous diagnostic — exact home=-1 trigger captured; capture ended

0.7.3 diagnostic build (a864a5e), loaded build-v7d-identity-log, stopped at7909:
**person120842, world1, expected_home75531, observed_home-1, destinations
[-1,78804,-1], target78804 unchanged, removedmask0**. Four switches/boardings;
three original arrivals:122261→72934 at2444/2445;40365→86033 at4537/4540;
123348→72220 at7371/7372.120842 arrival unobserved. No fifth switch, lifecycle
resolution or resumption; continuation is still unproven. query_errors0.

Watcher89335 exited0 at17:34:21.006802Z. A read-only copied follow-up started
17:35:12.134176Z (~51sec later) and immediately confirmed120842 is now only
EcsOctreeNode in BOTH worlds. Original75531/78804 still buildings. This does NOT
measure removal latency or prove it fits32 callbacks, nor attribute natural cause.
All reader/test/Ghidra/watcher processes ended. User told game may be paused/closed.
Original DLL remains mapped but forwards only. Never inject a second build here.

Last process Linux82918/start1979449 / Windows308, worlds1/2
0x37a25410/0x37a260d0, thread592; expire on restart/reload. Metadata live-run.json.
Evidence under ignored build-v7d-identity-log:
- log19903924 SHA256 b2af1ec92d8a2dca87979468f4062864126ddb5b76ac06f36695a2139b7f43f2
- home-clear-followup.jsonl SHA256 30b7de26d12e62f0b17027b71005d42aab64f152e3fcd4b2c048c397d6597b75
- live-audit.json SHA256 f544d744a157fd8f28ed57772b892493a25db764cf246c4da6a3fccae4d01673

**Next: live-test prepared0.7.4**, commit4d6e70a already pushed, build-v7e NOT loaded.
Requires fresh game restart and correct before_multi_passenger baseline. No new
restart has been requested yet. Validate home-pending→both-world removal within
32 callbacks→a later different-person switch. Existing policy/limits and reserve
remain. Do not expand grace/cap automatically. Full details in
[home-clear handling](../../docs/home-clear-handling.md) and continuation guide.
The historical running sections below describe startup only; capture is FINISHED.

## Cleared-home implementation prepared — NOT loaded

Version0.7.4 build-v7e adds a32-callback pending state for exactly home=-1 after
both-world boarding, with original target/slots retained and no arrival or
invalidation evidence. New switches pause; only both-world removal resolves it.
Restoration/reassignment, mixed evidence and deadline expiry still stop. No cap
refund, ID rebasing, query/ABI/mutation/route changes. Normal/reserved fixtures,
host batch/failure and both DLL contract suites pass; peak40/64 queries.
Mutable DLL7f136927850b…; full hashes/evidence in
[home-clear handling](../../docs/home-clear-handling.md). Build-v7e is NOT loaded.

Static function140b33470 explicitly removes when first destination(home)=-1.
This supports the bounded observation policy but does not prove the previous
identity stop's exact person/value. Diagnostic run selected40365 instead
of120770; it later caught the same guard for120842 with home=-1 (latest result
above). It used0.7.3 only;0.7.4 remains fixture-tested.

## Identity diagnostic startup (historical; finished above)

User restarted and loaded the correct before_multi_passenger baseline paused.
Fresh Linux82918/start1979449 / Windows308. Stdout load17:05:36Z, ready17:06:27Z;
no previous bridge mappings. Loaded build-v7d-identity-log DLL SHA256
`95e3557e502cd5237cf4417896163283c3c0b171d853e07462c4438fd08a07d4`
(source a864a5e), tfx_start0. Log `tpf-bridge-308-19903924.log` confirms cap5/18000,
reserve1. Adjacent live-run.json. **User confirmed normal-speed unpause.**
Fresh world1 `0x37a25410`, world2 `0x37a260d0`, thread592. Log watcher89335.
External monitor65191 ended exit0 at600sec,17:18:36.890100Z (11,271 world
samples/13 rejections).120770 lost SimPerson with home87062 Scaffold in both
worlds at17:14:22Z, WITHOUT being switched in this run. No transient home value
was captured. Identity-transition.jsonl hash1769b6c3412e… recorded in new guide.
Native watcher89335 has since finished as recorded above. Never reuse prior process/worlds below.

This build adds precise identity-stop logs only; all policy/limits unchanged.
Monitor pilot_identity_error for passenger/home/sequence/world, then preserve
post-stop read-only inventory. Do not infer missing home equals completed removal.
Prepared revised policy above is not loaded; no live continuation proof yet.
First three passengers120842/122261/40365 boarded at514/520/1095.122261 arrived
at original72934 at2444/2445. Original build-v7d preserved.
Targeted Ghidra export home-transition-dispatch.txt confirms caller14256b0c0
invokes ApplySimPersonData14256da20; it does not identify this event's runtime cause.
Ghidra process exited0, no debugger attached.

## Lifecycle continuation trial stopped — identity guard (latest)

Version0.7.3 (`6cab818`, startup `0daf9c8`) loaded correctly from build-v7d.
Fresh Linux79516/start1873255 / Windows308, thread800; worlds1 `0x2d468450`,
2 `0x2d467790`. User unpaused, then paused after stop; game left open. These
addresses expire on restart/reload. Original DLL remains loaded but forwards only.

Four successful distinct switches and both-world boardings:120842/122261 S3→S1
97745 at425/429;120770 S1→S3 97912 at1104;122790 S1→S3 97914 at1477. Then
`passenger_identity_unresolved` after observation2100. Query_errors0; all four
arrival/removal/invalidation masks0. No fifth attempt, lifecycle resolution or
resumption. **Continuation remains unproven; no original arrivals verified.**
Watcher47017 exited0 at16:59:12.566660Z. No watcher/readers/tests remain active.

Paused read-only inventory:120770 now EcsOctreeNode-only; other three retain
original destination assignments. Source guard plus zero removal masks implies
some still-present SimPerson's home changed, but old log lacks person/home/sequence.
120770 is plausible, not proven. Do not equate later absence with a resolved
pilot event or bypass the identity guard. `switching_blocked=0` is a separate
latch field; pilot_done and forwards-only message confirm no more changes.

Evidence under ignored build-v7d: log18964006 SHA256
`0832536ca8f81995a781809b11bdf765f9198f966f29ff4806a3ccbd865da812`,
identity-stop-inventory.json SHA256
`2e951d4fe6a541b4b8934cb86a53dee45c44120ecc0408cf554ba87f30bd6d21`,
live-audit.json (log invariants passed, continuation false), live-run.json.
Baseline before_multi_passenger remains ce2e02d935da…; no saves overwritten.

Added diagnostic-only pilot_identity_error (person/sequence/world/home/slots/
target/removal mask) before unchanged stop. Normal/reserved consumer fixtures
pass. Separate build-v7d-identity-log DLL SHA256
`95e3557e502cd5237cf4417896163283c3c0b171d853e07462c4438fd08a07d4`
**not loaded**; tests evidence identity-log-tests.json adjacent. Do not reinject
in current process. Loaded original DLL SHA2566c29dc158048… remains intact.

Next capture exact home transition before deciding a bounded pending-lifecycle
policy; keep unexplained identity changes/reuse as stops. No further restart/reload
requested yet. Full scope/evidence: [continuation guide](../../docs/lifecycle-continuation.md).
Five attempts/18,000 callbacks with fifth reserved for classified lifecycle;
S1↔S3 stops1→2 only. No stable release or broad route/preference expansion.

## Lifecycle diagnostics — live trial completed (latest)

Version0.7.2 implementation `935ef65` and startup/arrival checkpoint `45fafcb` are
pushed. Build `analysis/native-bridge/build-v7c`, mutable DLL SHA256
`2f8ca09d693e9411327660a730e4278e17347e012e65cc0a4a0f3da3e3127ca0`.
**Five distinct switches/boardings, four original arrivals, one both-world removal.**
At7670 the consumer stopped with `reason=passenger_removed`, query_errors0 and
switching_blocked1. Final wrapper log says forwards only; no further changes.
Log watcher session16151 finished exit0 at15:17:46.912452Z; all external readers
have exited. User was told they may pause. No new saved checkpoint requested.

Passengers:120842 S3→S1/97745, removed before confirmed78804 arrival;122261
S3→S1/97745 reached72934;40365 S1→S3/97912 reached86033;72701 and73064
S3→S1/98594 reached72691/73063. All substituted legs alighted at planned stop2;
onward transfers were observed. Arrival sequences and limits are in the
[complete trial record](../../docs/lifecycle-tracking.md).

After122261's workplace arrival, independent copied reads confirmed its work
slot cleared and target became its existing home122255. The final arrival mask
correctly remains3. Person120842's missing SimPerson at7669/7670 was independently
corroborated by an EcsOctreeNode-only inventory. **Its specific removal cause is
not traced**; it differs from120770 in the earlier no-mutation control. Do not
call this five successful journeys or assume a natural cause for this new event.

ABI7 query/pilot/host/mutation/API suites passed; peak fixture allowance40/64.
Live test covers removal classification and retaining completed arrivals through
later invalidation. Pre-arrival invalidation remains fixture-only. Attempt cap5
was exhausted before removal, so fixtures (with allowance remaining) establish
the new permanent block independently. Policy is still S1↔S3 stops1→2 only.

Historical process: Linux64386/start1139296 / Windows308; thread776,
world1 `0x6748b7d0`, world2 `0x6748ab10`. These expire on restart/reload. DLL stays
loaded until exit. Do not attach a second build or restart this completed capture.
Log `build-v7c/tpf-bridge-308-11756348.log` SHA256
`64ff0426fc8bb1bd8514639fd263c4fa91d475af295e939679ac87e1203c5f69`.
Metadata/evidence: `live-run.json`, `live-audit.json`, `after-arrival-lifecycle.json`,
`final-entity-inventory.json`, `last-vehicle-progress.jsonl`; full hashes in record.
Preserved baseline remains ce2e02d935da…; no save files were changed by our tools.

Next implementation can build on these explicit outcomes. Keep unknown read and
identity failures as hard stops, separate lifecycle censoring from arrival, and
retain route/attempt guards. Continuing after classified outcomes or expanding
routes requires a distinct bounded change/test; neither is implemented here.
No stable release/autoload or measured journey-time improvement is established.

## Completed read-only lifecycle control (latest)

**Both lifecycle changes reproduced without interlining switches.** From the
preserved pre-trial baseline,120770 lost SimPerson in both worlds at476.55s,
concurrent with Scaffold appearing on its home87062. At560.06s,122261 was at its
unchanged home122255 with workplace72934 cleared (-1), while72934 gained Scaffold.
This supports native building redevelopment; exact native causality is not traced.
Neither case counts as original-destination arrival or proves all mod behavior safe.
See [lifecycle investigation](../../docs/passenger-lifecycle-investigation.md).

External monitor session2247 **finished, exit0**, at2026-10-08T14:31:42.771544Z:
2,153 accepted world samples,7 rejected inconsistent reads. No external watcher
remains. Ghidra session75896 also finished successfully. User was told they may
pause or close the game; no further running time is needed for this control.
Only read-only `lifecycle-v7b/tpf_journey_observer.dll` was loaded. It may continue
to its3,600-callback cap and stays loaded until process exit. No mutation DLL.

Historical process for this control: Linux52616 / Windows308, thread628,
worlds1 `0x2bd709c0`,2 `0x2bd6fd00`; all expire on restart/reload. Baseline copy
`TPF3Interlining_before_multi_passenger_01` has source hash ce2e02d935da….
Artifacts under ignored `analysis/native-bridge/lifecycle-v7b/`:
`pre-trial-inventory.json`, `control-lifecycle.jsonl`, immutable
`control-observer-at-completion.log`, `control-run.json`, `watch_control.py`.
Full hashes and timeline are in the investigation document. Preserve both save
backups; do not replay this control merely to recover context.

**The lifecycle-classification implementation described above now addresses this
next step**, with fixtures separating original arrival, removed passenger,
invalidated destination and unresolved identity/read failures. Mutation limits
and eligibility remain unchanged. Missing state alone cannot justify more switching;
a lifecycle outcome is not a successful journey. Then plan a fresh bounded
trial with explicit handling of confirmed lifecycle outcomes. Other two batch
original destinations remain unverified. No stable release/autoload yet.

## Historical checkpoint — multi-passenger trial stopped for lifecycle investigation

Version 0.7.0 implementation `cb7c91d` and startup `a19a76b` are pushed on dev.
Loaded `analysis/native-bridge/build-v7a/tpf_shared_bridge.dll`, passenger cap5,
callback cap18000, ABI6 unchanged. Log `tpf-bridge-312-7795382.log`.
**Four distinct passengers switched and boarded in both worlds**, no repeated
attempts:120842/122261 S3→S1 vehicle97745;120770 S1→S3 vehicle97912;122790 S1→S3
vehicle97914. All original stops1→2 Wimborne→Melksham. Ages/destinations/place
bytes preserved. See [full trial evidence](../../docs/multi-passenger-trial.md).

**Capture auto-disabled after eight journey TFX_NOT_FOUND(6) reads**, following
callback2118, before a fifth switch. Exact stop sequence was not logged. All four
boarding masks3, destination masks0. No watcher remains; the DLL forwards only.
User paused and saved, reports no vehicle/line edits during capture. Current
Linux26846 / Windows312, worlds1 `0x2d1de790`,2 `0x2d1df450`, thread592; these are
historical inputs once the game restarts/reloads, never blindly reuse them.

Read-only follow-ups found two unresolved lifecycle observations:
- Person120770's entity still has components and destination72649 exists, but
  simulation-person, waiting, onboard, moving and at-building components are
  absent in both mask and row handles. Cause is unknown; do not label normal
  retirement, arrival or mod corruption without evidence.
- Person122261 transferred naturally via line79460/vehicle123296, then appeared
  at-building flags8 with target/at-entity122255, not original destination72934.
  Building identity/lifecycle mapping is unverified; do not claim arrival.
Other two remained readable:120842 boarded S1vehicle98610 stops4→3 toward78804;
122790 boarded S2vehicle98301 stops2→3 toward86009. Three120sec external readers
exited3 (no original-destination proof), all closed. Prior completed single-person
journeys remain valid historical evidence, not proof of this batch's endpoints.

Preserved expanded pre-trial baseline:
`analysis/baselines/ce2e02d935da/TPF3Interlining_multi_passenger_01.sav`
(80,431,012bytes; fullhash in trial document).
Diagnostic backup:`analysis/baselines/acca331ade37/TPF3Interlining_multi_passenger_diagnostic_01.sav`
(81,236,125bytes;sha256`acca331ade37c604be598cd694e28cf15fc7e8bdef25ff5b71ae6e7be5b86219`).
The user saved under the existing source name; backup copies preserve BOTH states.
Adjacent manifests record stable source timestamps and verified hashes.

Version 0.7.1 improves diagnostic logs only: person/destination/sequence on journey
errors; external snapshot helper reports entity presence and known component
mask/row agreement. Built `analysis/native-bridge/build-v7b`, **DLL NOT loaded**.
Consumer regression passed after diagnostics change; new helper also exercised
read-only against the missing entity. Do not overwrite loaded build-v7a artifacts.

**This was the investigation gate before the completed control above.** Compare native lifecycle behavior using preserved
baseline/checkpoint. Keep missing-record stop, bounds and eligibility unchanged;
do not count absence or unmatched at-building ID as successful arrival. No fifth
switch, complete batch journey validation, stable release or autoload is proven.

## Previous outcome and recovery

**Save/restart validation completed successfully.** Passenger **106847** switched
from S1 to S3 over Wimborne→Melksham, was saved onboard, retained that state after
restart, transferred naturally to S2, and reached original destination **85882**
in both engine worlds. Only read-only observers ran after restart. This is the
second completed alternate journey; the first was person 72845 without a reload.

The user reported a power cut after the final observation. Recovery verified the
arrival log and exit-0 metadata, both saved checkpoint hashes/sizes, `git fsck
--no-dangling`, and `git diff --check`. Source milestones were already committed
and pushed; remaining work was final documentation. **No test replay, active
watcher, game restart or user action is needed to finish this milestone.** Do not
reuse historical process/world addresses or resume an old capture.

Full evidence and hashes:
[repeat/save-reload trial](../../docs/save-reload-trial.md).
First journey: [mutation trial](../../docs/native-mutation-validation-2026-10-08.md).

## What the completed trial observed

- 0.6.4 pilot at `analysis/native-bridge/build-v6e/`, configured for 18,000 callbacks.
  Log `tpf-bridge-312-6954251.log`; source implementation milestone `49ee290`.
- At callback **1119**, person **106847**, S1 **98278** stops **1→2**, changed to
  S3 **97743** vehicle **97913**, stops **1→2**. Destination **85882**, 39 free
  places, boarding **97948:1**, alighting **97294:0**. One engine attempt, preserved
  arrival bits **0x2275e0**, destination and waiting place. Both-world boarding.
- User paused and saved **TPF3Interlining_after_switch_01**. Matching external
  reads around the save handoff confirmed onboard state in both worlds.
- After full restart, only `reload-106847/tpf_journey_observer.dll` was loaded.
  First callbacks confirmed saved S3 line/vehicle/stops/destination in both worlds.
  Log `tpf-bridge-304-7442601.log`, observer capabilities **95**, no mutation backend.
- Callbacks **1508–1510**: left S3/onward movement. **1689–1690**: waiting for
  S2 **97742** stops **2→3**, destination unchanged. Observer auto-disabled at
  **3600**, zero query errors/rejected boundaries and zero passenger changes.
- External read-only follow-up **02** recorded boarding S2 vehicle **98390** in
  both worlds; **04** recorded onward movement; **05** recorded at-building
  `flags=8`, target/at-entity **85882**, in both worlds. Final summary:
  `samples=413 destination_world_mask=3 observational_only=1`; exit **0** at
  **2026-10-08T11:19:57.272457+00:00**. The wrapper stopped without windows 06–08.

External snapshots are observational copied reads, not atomic transactions.
The transfer-platform scan was partial (one unavailable world, other not idle at
end); a later service scan covered one world. Those are explicitly labeled and
are only corroboration. The user did not see an alighting animation; no visual
verification was claimed. Native state and final arrival are the test evidence.

## Preserved artifacts

All paths below are ignored research artifacts, not files to commit.

- Warm-up: `analysis/baselines/04569232cb31/TPF3Interlining_warmup_01.sav`,
  77,688,477 bytes, SHA-256
  `04569232cb31adfdec45d83745b21aeb9179944d745c436be6cc6897414644d5`.
- Onboard checkpoint: `analysis/baselines/f1b92fb2ec15/TPF3Interlining_after_switch_01.sav`,
  78,144,391 bytes, SHA-256
  `f1b92fb2ec1546ce305118464c6ab8f2faab796aedeba06f5efa93fef34146b1`.
  Source path, timestamp and copy verification are in the adjacent manifests.
- Loaded pilot `build-v6e/tpf_shared_bridge.dll` SHA-256
  `5a649b0aa32b63f35f5d88646136452b6b837fb2d8cfb1cffc8bba414622c657`.
- Observer `reload-106847/tpf_journey_observer.dll` SHA-256
  `4c904a5ab5b04c9bb0f256a7af6660d638fead19f17976ddda885765b4592662`.
- Arrival `reload-106847/followup-05.log` SHA-256
  `884ef7510df8b3009611ef85ee7c72d7c8a8ea8571659eedab09b583701475af`.
  Native artifact paths are relative to `analysis/native-bridge/`. Full log hashes
  and earlier captures are recorded in the trial documents. Preserve loaded-build
  artifacts; always use a new build directory for changes.

## Policy scope clarified with the user

The restriction is to an **existing adjacent train leg**, not final destination
being within one station. Person 106847 already had S1 Wimborne→Melksham as their
planned next leg before switching. S3 replaced that same leg; native continuation
subsequently used S2. The full original onward itinerary was not captured, so do
not assert S2 was always the planned next connection or that time was saved.
No deliberate travel-ahead/wait-for-original-train policy was added.

A user-reported Wimborne example, person 75515 on S3 stops 1→4, was correctly
excluded: the planned alighting station was absent from S1. Through legs are not
shortened to manufacture transfers. Current pilot requires same accessible
boarding and alighting terminals; native walking after alighting is distinct
from implementing platform switching before the substituted boarding.

## Implementation and next step

ABI 6 adds deferred waiting-leg requests, owner-scoped results and copied journey
queries. APIs 1–5 retain layouts. All same-person request conflicts reject;
revalidation uses a fresh callback token. Backend compares the entire 32-byte
waiting component and public expected state, then invokes Begin/Remove/Add/End
before original terminal selection. The historical 0.6 implementation allowed one engine attempt globally per DLL
lifetime; version 0.7 adds bounded distinct-person attempts and a failure latch.
No rollback promise.

Bindings: Begin `0x142bb4640`, Remove `0x142bb7320`, Add `0x140b21cd0`, End
`0x142bb4d90`, waiting RTTI `0x143cea8f0`. **Add has five arguments**: world, person,
copied value, notify=1, local storage=1. Stack store at `0x140b3116d` before call
`0x140b3117e` proves the fifth argument. Full executable hash and live signatures
are checked. Do not use abbreviated Ghidra signatures or direct field writes.

0.6.2 introduced rolling discovery of at most 32 person IDs. Its regression passed;
the short low-demand live run remembered only 26, so it did not exercise eviction.
0.6.3 added optional read-only observer build inputs. 0.6.4 added
`--observation-limit` (pilot only; default 3600, range 1–36000) and periodic progress
logs. Observer cap remains 3600. SDK/mutation/query/candidate suites passed;
passenger-specific observer fixtures and actual binary contract checks passed.
Latest recovery edits are documentation only; no rerun of unchanged code needed.

The bounded multi-passenger implementation and four boardings are now recorded
at the top of this handoff. The current gate is lifecycle/identity investigation,
not another increase in passenger count. Keep eligibility expansion separate.
Native-client faults/hangs and multi-mod coexistence remain outside demonstrated
coverage.

Longer shared legs, loop wrap, alternative terminals, fractional/full-load rules,
line edits and broad save compatibility remain open. Task 007 preferences/time
comparison are later work; exposed fields do not establish individual cheap/fast
preferences. Continue on `dev`; no stable release or automatic installation exists.

## Working references and tools

- [Native SDK/build/loading guide](../../native/README.md)
- [Candidate policy and limits](../../src/interlining/README.md)
- [Mutation boundary and replication design](../../docs/native-adapter-design.md)
- [Shared extension contract](../../docs/shared-extender-design.md)
- [Read-only candidate evidence](../../docs/native-candidate-validation-2026-10-07.md)
- [Platform/nonzero-reservation evidence](../../docs/native-platform-validation-2026-10-07.md)
- [Vehicle layouts](../../docs/native-vehicle-validation-2026-10-07.md)
- [Component/query layouts](../../docs/native-query-validation-2026-10-07.md)
- [Native runtime/boundary evidence](../../docs/native-runtime-observation-2026-10-07.md)
- [Boarding/index investigation](../../docs/native-boarding-investigation.md)
- [Scripting-hook limitation](../../docs/scripting-hook-investigation.md)

Every shell command uses `rtk`; read `/home/vibeshibe/.codex/RTK.md`. Work on dev,
main for stable releases; user authorizes regular documented commits/pushes.
Git/network and Wine host access may need reviewed escalation. Origin libsecret
credentials work. Preserve existing AGENTS.md. No subagents authorized.

Build via `rtk proxy python3 tools/build_native_bridge.py --build-dir NEW_DIR`.
Toolchain pinned under `tools/vendor/`; isolated tests use Wine prefix
`tools/runtime/bridge-test`. Game loader must use GE-Proton11-5-x86_64's Wine and
prefix `/home/vibeshibe/Games/Heroic/Prefixes/default/Transport Fever 3/pfx/`.
Identify Windows PID with loader `--list` after every restart. Wrap Wine loader
calls in an outer 45-second timeout. The old optional stop-loader stalled once; auto-disable
is sufficient. A timeout is not proof a remote operation failed.

Ghidra 12.1.3 project `analysis/ghidra/TPF3Interlining`, Java 21; targeted exports work,
whole-program analysis was partial. Do not reimport. See tools/ghidra/README.md.
Latest ignored exports: `mutation-bindings.txt`, `mutation-world-dispatch.txt`,
`terminal-permissions.txt`, with earlier queue/lifecycle/modification exports.
No Ghidra MCP installed. Generated analysis, tools and game files stay ignored.
