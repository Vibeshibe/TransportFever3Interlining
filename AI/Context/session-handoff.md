# Session handoff — 2026-10-08

## Active work — five-passenger trial

Version 0.7.0 is prepared in `analysis/native-bridge/build-v7a`, ABI 6 unchanged,
`--passenger-limit 5 --observation-limit 18000`. Offline host/consumer/native tests
passed. See [the trial record](../../docs/multi-passenger-trial.md) for bounds,
commands, artifact hashes and remaining live validation. Expanded baseline backed
up under `analysis/baselines/ce2e02d935da/`; user reports 40+ waiting people and
confirmed restarted, loaded and paused. Preflight Linux26846 / Windows312 had no
old bridge mapped. Implementation committed/pushed as `cb7c91d`. Loader returned `tfx_start=0`;
`build-v7a/tpf-bridge-312-7795382.log` confirms five-passenger/18,000-callback
startup. User asked to unpause at normal speed; no Update observations yet at
this checkpoint. Monitor this log next; do not reload the DLL or claim live success.
The host now permits an explicit bounded count, with once-per-person ledger and
failure latch; older one-attempt descriptions below refer to completed 0.6 trials.

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
before original terminal selection. One engine attempt globally per DLL lifetime,
even on a postcondition failure; no rollback promise.

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

**Next implement a small bounded multi-passenger trial**, retaining fresh policy
checks and guarding against repeated selection of a passenger across the two
worlds. The current host still permits only one attempt: do not claim multiple
changes in one process are tested. Keep eligibility expansion separate from
operation-count changes. Native-client faults/hangs and multi-mod coexistence
remain outside demonstrated coverage.

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
