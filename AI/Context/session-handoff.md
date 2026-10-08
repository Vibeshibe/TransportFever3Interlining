# Session handoff — 2026-10-08

## Latest repeat capture: complete, no changes

The 0.6.2 pilot loaded into Windows PID 312 (`tfx_start=0`); user unpaused.
Log: `analysis/native-bridge/build-v6c/tpf-bridge-312-5551718.log`. It completed
3,600 callbacks, 28 candidate evaluations (one positive return-leg candidate,
seven unsupported legs, twenty boarding mismatches), zero query errors/rejected
boundaries, and **zero staged requests/engine attempts**. It is now forwarding
only; the DLL remains pinned until process exit. There is no active watcher.

User clarified the original baseline has young routes and sparse demand and
asked to give it more time. Prepared 0.6.4 longer capture below. User was asked to
save current progress as `TPF3Interlining_warmup_01`, restart, load it and pause;
this action has not been confirmed. Do not discard simulation progress.

After auto-disable, user reported S1 arriving at Wimborne with an S3 passenger.
Read-only external snapshot found person 75515 waiting on S3 stops 1→4, destination
86142, platform 97948:1. S1 vehicle 98594 was at stop 1/platform 1 with 40 free
places, load_state=4. S3's intended stop 4 (station 97699) is absent from S1's
route: this case requires onward replanning/transfer, outside the current pilot.
User acknowledged the distinction. Only world 1 was readable; world 2 failed
idle/system preconditions. No passenger or game state was changed. Evidence and
hashes: [trial record](../../docs/save-reload-trial.md).

## Previous completed result

**First actual interlining switch succeeded.** Revision 0.6.1 applied one
engine-owned S1→S3 waiting-component replacement and then observed the passenger
onboard the replacement vehicle in both engine worlds. The passenger then alighted
at the planned stop and reached the original destination, observed in both worlds. This is a disposable-save development
trial, not a stable release or an automatically installed mod.

Read [the mutation trial record](../../docs/native-mutation-validation-2026-10-08.md)
for evidence, hashes, native call signatures and test results. The user restarted,
loaded a disposable baseline paused and then unpaused at normal speed. That trial
loaded artifact was `analysis/native-bridge/build-v6b/tpf_shared_bridge.dll`,
Windows PID 312, log `tpf-bridge-312-3397243.log` beside it. DLL remains pinned
until exit. Do not overwrite it or load a different bridge into the same process.
The capture is complete and forwarding only. User was told they can pause/close.
No debugger is attached. No game/probe/save files were edited by the tools.

At sequence 2473, person **72845**, S1 `98278` stops **1→2**, destination **72405**,
qualified for S3 `97743` vehicle **97914**, stops **1→2**, with 39 places remaining.
Boarding station 97948 terminal 1; alighting station 97294 terminal 0. Ticket 1 applied
in world `0x32cd01e0`, preserving arrival bits `0x461ba8`, destination and waiting
place. Onboard state (`flags=2`) followed in world ID 2 (`0x32ccf520`) then world ID 1
(`0x32cd01e0`), thread 592. Only one engine attempt is permitted per DLL lifetime.

The injected capture completed **3,600 callbacks**, one engine attempt, no query
errors and zero rejected boundaries. It observed leaving S3 at current stop 2
(the planned alighting stop) and then onward movement (`flags=4`) in both worlds.
A separate external read-only `journey_snapshot` then observed at-building state
(`flags=8`) at original target **72405** in both worlds. The follow-up log is
`analysis/native-bridge/build-v6b/journey-followup-72845.log`; exact helper retained
as `journey_snapshot-captured.exe`. The bridge was not re-enabled. Full hashes
and observation limits are in the trial record.

## Implementation and prepared revision

ABI 6 adds staged `request_waiting_leg_change`, owner-scoped result tickets and
`read_person_journey`. ABIs 1–5 retain layouts/capabilities. Requests are copied
until observers finish; all same-person conflicts reject. Consumer revalidation
uses a fresh token and repeats current route/platform/permission/capacity checks.
The backend compares the full 32 native waiting bytes and public expected state,
requires waiting-only state, and invokes Begin/Remove/Add/End before original
terminal selection. Only three route integers change. One engine attempt is
allowed globally, including a postcondition failure; no rollback promise.

Bindings: Begin `0x142bb4640`, Remove `0x142bb7320`, Add `0x140b21cd0`, End
`0x142bb4d90`, waiting RTTI `0x143cea8f0`. **Add has five arguments**: world,
person, copied 32-byte value, notify=1, local storage=1; stack store at `0x140b3116d`
proves the fifth argument before call `0x140b3117e`. All four live entry signatures
are checked, alongside the pinned executable hash and terminal vtable/slot.
Do not use Ghidra's abbreviated caller signature or direct waiting-field writes.

The first 0.6.0 trial (`build-v6`, PID 308) found valid S3→S1 candidates but its
one-direction pilot filter excluded them. It ended on restart with no engine
attempt. Source commit `6abfdc3` implements mutation; `c819e21` permits either
S1/S3 direction on the same adjacent shared segment. Both were pushed to dev.

**Now loaded for the repeat trial:** revision 0.6.2 at `analysis/native-bridge/build-v6c/`.
Code review found the observer permanently retained its first 32 person IDs.
`pilot_cache.h` now replaces older discovery IDs as new people arrive and clears
log suppression for replaced slots. All eligibility reads remain fresh. A 96-ID
regression and candidate policy tests pass; the warnings-as-errors build passes.
The host, mutation operations and eligibility policy are unchanged. Do not claim
this revision was used for the successful live switch; that was 0.6.1.

All three native suites passed before the trial: `bridge_tests`, `query_tests`,
`candidate_tests`. New fixtures cover deferral, two-client conflicts, result
ownership, thread/token guards, stale bytes/destination, policy refusal, scope
preconditions, call order/fifth argument, preserved bytes, postconditions, attempt
limit and copied journey states. API journey flags: waiting=1, onboard=2, moving=4,
at-building=8. Person +0x50 is target/at-entity; onboard line/stops/vehicle at 0/4/8/12.
Movement/building flags use masks, not guessed component layouts.

## Save/reload preparation after the first trial

Revision 0.6.3 adds an optional `tpf_journey_observer.dll` for a known passenger
and destination. It discovers worlds afresh after restart and has no mutation
backend. Build inputs are paired `--observe-person` and `--observe-destination`;
see [the checkpoint procedure](../../docs/save-reload-trial.md). The initial
`build-v6d` observer uses the previous passenger's IDs for offline validation only.
Rebuild in a new directory using the repeat trial's actual IDs before loading it.
The repeat write trial still uses the preserved 0.6.2 `build-v6c` artifact. All four
offline suites passed, including the observer binary contract and read-only host
regression. Observer and test-log hashes are in the checkpoint procedure.

Before loading the repeat pilot, host inspection found Linux PID 33922 / Windows
PID 312 without a bridge mapping. The user then confirmed the save paused and
authorized loading. See the active-trial section above. PIDs can be reused.

## Longer capture prepared, not loaded

User clarified the original baseline has young routes and low demand; give the
simulation more time and retain eligibility rules. Revision 0.6.4 builds a longer
pilot with `--observation-limit 18000` at `analysis/native-bridge/build-v6e/` and
logs every 300 callbacks. The loaded 0.6.2 DLL has stopped at 3,600. No live cap or
memory was patched. Preserve the progressed save before restarting into it with the longer capture. That warm-up save is not a
switched-passenger checkpoint. The return-leg candidate is evidence only; the
pilot's original stops 1→2 selection and one engine attempt remain unchanged.

## Next steps

1. Wait for the user to confirm saving/restarting into the progressed baseline,
   then load the 0.6.4 `build-v6e` pilot and observe for an onboard checkpoint.
   Preserve the historical 0.6.1/0.6.2 DLLs and capture logs.
2. Verify save/reload and repeat controlled replacements before broader rollout.
   No generic boarding/entry/platform trace needs repeating; those facts are proven.
3. Broaden from the adjacent same-terminal leg only after correctness is established.
   Longer shared segments, wraparound, express stops, alternative terminals,
   fractional/full-load rules and node-fallback access remain excluded.
4. First implementation uses the passenger's current accessible waiting location;
   walking to another platform is future work requiring native path/timing checks.
   User offered a separate S2 shared-platform save. Keep current S2 platform
   difference as a negative control; retain the current baseline network.
5. Preferences remain task 007. Exposed passenger fields do not establish a personal
   cheapest/fastest preference. Ticket/comfort data and movement-mode cost factors
   exist, but monetary route weighting and individual variation are unverified.

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
