# Native cross-platform boarding — 0.10.0

## Behavior and scope

A waiting passenger may now switch to another line loading at a different
platform **inside the same station entity**. The train must be locked to its
configured primary platform or an explicitly configured alternative. A shared
station group alone does not authorize boarding at a separate station building.

The existing engine-owned waiting-leg replacement preserves the passenger's
waiting place, original destination and waiting age. Native loading then performs
the vehicle approach or direct boarding fallback. The mod adds no walking delay,
seat reservation or forced onboard assignment. This is an experimental use of
the native behavior established by [static research](alternative-platform-behavior.md);
cross-platform boarding and onward arrival still need live validation.

The original alighting **station** must still match. Its fixed platform may
differ; configured alternative terminals at that future endpoint remain excluded. One-to-four forward intervals, ordered
express skips, capacity/reservation checks, terminal loading permissions and
fresh deferred validation remain. The actual replacement alighting terminal must permit passenger unloading. No new interchange
or whole-trip cost optimization is introduced.

## Shared API and implementation

ABI 8 adds `read_stop_terminals`, returning a copied line stop and up to16 resolved
terminal entries, primary first. Each entry has station index, station entity
and terminal index. Older ABI layouts/capabilities remain available. Oversized
lists fail completely; the consumer records `terminal_limit` rather than guessing.

The reader reuses the existing stop query's metadata and one query-budget unit.
Stop registration `0x1418829c0` identifies `alternativeTerminals` at offset0x10;
StationTerminal registration `0x1422c1ab0` binds station/terminal at offsets0/4.
The reader copies 8-byte pairs and resolves indices through the current world's
station-group vector. Ignored exports `alternative-terminal-layout.txt` and
`alternative-terminal-element.txt` under `analysis/native-boarding/` record this
build-specific evidence; executable hash is unchanged from the research guide.

Candidate validation requires waiting-edge membership in the original station,
checks loading permission at the **actual selected terminal**, and retains all
configured boarding options plus the selected terminal in its deferred snapshot.
Alighting-path handler `0x140b25e30` resolves the onboard line/stop and constructs
a new path toward `SimPerson.targetOrAtEntity` through `0x140952920`/`0x140b378e0`
(see `analysis/native-boarding/transition-callbacks.txt`). This supports a bounded
trial of different fixed alighting platforms; onward reachability and arrival
remain live-test outcomes, not guarantees from station identity alone.
A changed platform selection or configuration invalidates the staged proposal.
No engine pointers or new mutation primitive cross the shared SDK.

Logs add `platform_policy=native_same_station`, `cross_platform` and
`alternative_platform` and `cross_platform_alighting`. The former candidate flag compares the original line's
configured boarding terminal with the selected terminal; it does not measure a
passenger's physical walk. The latter identifies selection away from the
replacement line's primary terminal. `board=station,terminal` is the actual
resolved selection; the alighting flag compares the two fixed endpoint platforms.
Native indices are zero-based; UI platform3 may be terminal2.

## Build, validation and delivery

Prepared build directory: `analysis/native-bridge/build-v10a`. The test package
is under that directory as `TPF3Interlining-0.10.0-trial.zip` with DLL,
loader, instructions and a checksum manifest. It uses32 lifetime attempts,
four detailed journey samples, one pending boarding and18,000 callbacks. Every
switch still requires both-world boarding confirmation; unsampled completion
does not imply final arrival. This is a development package, not an automatic
installation or a stable `main` release.

```sh
rtk proxy python3 tools/build_native_bridge.py \
  --build-dir analysis/native-bridge/build-v10a \
  --passenger-limit 32 --journey-sample-limit 4 --observation-limit 18000
```

Fixtures cover primary/cross-platform boarding, explicit alternatives, unrelated
stations in a group, unlisted/unlocked arrivals, actual terminal permissions,
different fixed alighting platforms, preserved endpoint stations/destination,
unloading restrictions, configuration/selection changes before mutation,
malformed/oversized/shared query storage and callback/thread/budget guards.
The consumer's worst fixture allowance remains63/64 queries. Final artifact
hashes and test results are recorded in the package manifest and progress log.

## One integrated S2 trial

Finish or intentionally stop the existing0.9.1 run, then restart the game to
remove its DLL. Load a disposable baseline retaining S2 at platform3 and leave
it paused. Use a fresh Windows PID, the game's Proton version/prefix, and the
existing [loader instructions](../native/README.md#explicit-loading-under-windowsproton).
Load only the new DLL and check startup0.10.0/ABI8 before unpausing normally.

Look for a passenger waiting for S1/S3 who boards S2 at the other platform;
the planned alighting station must match. The user reports S2 uses platform2
in Faringdon and platform3 elsewhere, so this fixture exercises differences at
both ends. These names and platform numbers are test data, not hard-coded rules. Record station, direction,
vehicle and approximate time. Logs must show `cross_platform=1`, successful
replacement and both-world boarding, then sample the original destination.
An S2 boarding at its fixed platform tests cross-line boarding; a train actually
using a configured alternative is a separate coverage case.

Leave lines/vehicles unchanged during capture. The bridge stops changing
passengers at its callback/attempt limits but does not pause the game. Logs sit
beside the loaded DLL. If boarding is unconfirmed or an invariant fails, preserve
the log/checkpoint and investigate; do not raise limits or declare success from
a proposal alone. Restarting without the DLL removes the active extension;
load the untouched baseline to undo saved passenger changes.
