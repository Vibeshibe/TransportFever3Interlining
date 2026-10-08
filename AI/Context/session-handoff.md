# Session handoff — 2026-10-08

## Lifecycle continuation implemented — ready for fresh live test (latest)

User asked to finish implementation/automated tests before another game load.
Version0.7.3 is complete in `analysis/native-bridge/build-v7d`, **not loaded**.
Mutable DLL SHA256 `6c29dc15804889f319f471d81ff5b04f25b15a5047b51bc915d0b6562609b39c`.
ABI7/query/mutation implementations unchanged. Five attempts/18,000 callbacks,
same S1↔S3 stops1→2 eligibility; trial reserve enabled for the fifth attempt.

After both-world boarding and matching lifecycle confirmation, the affected
passenger closes without an arrival or attempt refund; other passengers may be
served once ALL pending lifecycle cases resolve. Any permanent query-error block
remains. Unknown identity/reassignment/disagreement and lifecycle-before-boarding
still stop. Closed IDs are never reselected or read again. Final mixed batch
reason is all_outcomes_observed, distinct from all_destinations_observed.

Normal/reserved consumer, host batch/failure, candidate and both DLL contract
suites passed. Max fixture allowance40/64. Invalid reserve/cap1 rejected by CLI
and CMake. No running test/watcher sessions. See [continuation guide](../../docs/lifecycle-continuation.md).

Next: request readiness for one fresh restart/load of disposable
`TPF3Interlining_before_multi_passenger_01` paused. Its ce2e02d935da… hash was
rechecked unchanged. Previous0.7.2 may remain loaded until game exit. Verify
fresh process/save, load only build-v7d mutable DLL, then ask to unpause. Require
confirmed lifecycle resolution and a subsequent successful different-passenger
change with remaining allowance; a resumption log alone is insufficient.
If no lifecycle event occurs, cap expiry is inconclusive—do not expand it silently.
Do not reuse historical PID/worlds below or overwrite loaded DLLs.

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
