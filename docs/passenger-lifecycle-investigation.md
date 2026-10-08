# Passenger lifecycle investigation — 2026-10-08

## Questions and current evidence

The four-passenger trial ended on missing journey state for ID120770. A second
passenger,122261, later had at-building target122255 rather than original72934.
The trial's original-destination completion remains unproven. A subsequent
read-only control reproduced both lifecycle changes without interlining switches;
see the completed control below. This does not count either journey as completed.

A read-only restart into the diagnostic checkpoint confirmed:

- **120770 is now a scenery asset ID**, with ModelInstanceList, BoundingVolume,
  AssetGroup, AssetGroupAutoRemove, EmissionEmitter and OctreeSystemNode components.
  No SimPerson or journey components remain. Numeric entity presence alone cannot
  establish continued passenger identity. The removal/reuse cause is not captured.
- **122261 remains a SimPerson**, with SimEntityAtBuilding. Its current target is
  **122255**, matching the first (home) entry in destinations **[122255,75501,-1]**.
  Both72934 and122255 currently have TownBuilding and PersonCapacity components.
  This does not establish that the original72934 target was visited, nor that the
  two IDs denote the same building. The pre-trial comparison below confirms that
  122255 was already its home.
- **120842** remains a passenger with target78804 and destinations[75531,78804,-1].
  **122790** remains a passenger with target86009 and destinations[101879,86009,-1].

Both world inventories agree on these component sets/values. The external reader
requires repeated copied snapshots to agree and closed modification windows.
This is observational evidence, not an atomic transaction across worlds.

## Read-only capture and artifacts

User loaded the diagnostic save paused, briefly ran about five seconds for world
registration, then paused. Linux PID46033, Windows PID304, simulation thread616;
world1 `0x296154f0`, world2 `0x29614830`. Image was relocated to `0x6ffff9230000`;
never assume preferred PE image base for live reads. These addresses expire on
restart/reload. Loaded only `lifecycle-v7b/tpf_journey_observer.dll`, built for
person122261/original destination72934, capabilities95, no mutation backend.
Observer binary contract/unsupported-build test and tracking fixtures passed.

Paths below are relative to ignored `analysis/native-bridge/`; SHA-256:

- `lifecycle-v7b/tpf_journey_observer.dll`:
  `c0755d2e2cb89c9146203dc22058933b533d5b5433843b4f51c57cf49fa29c70`.
- `lifecycle-v7b/tpf-bridge-304-9304095.log` (at paused inspection):
  `ecd925a7eabe769c0fa6e986e213007453718edc257daf83e9c2c98ee9b53163`.
- `lifecycle-v7b/entity-inventory.json`:
  `09354175865b33a91e8814f2eca07014dfbd7a847a9b2cee5e89baae0675271a`.
- `lifecycle-v7b/post-trial-inspection.json`: process/build/checkpoint metadata.

The new `tools/entity_snapshot.py` inventories at most16 requested entities via
read-only `/proc/PID/mem`, with pinned on-disk executable hash, actual PE mapping,
4MiB read budget, bounded type/row/mask/storage vectors, row/mask consistency,
repeated snapshots and process start identity checks. It never calls game code.
It reports RTTI names and the established SimPerson target/destination-vector
layout, including shared pages. Paused closed-window flags0/1 are accepted and
recorded; this does **not** relax the stricter native mutation boundary.

```sh
rtk proxy python3 tools/entity_snapshot.py FRESH_LINUX_PID \
  --worlds FRESH_WORLD1_HEX FRESH_WORLD2_HEX \
  --entities 120770 72649 122261 72934 122255 120842 122790
rtk proxy python3 -m unittest discover -s tests -p test_entity_snapshot.py -v
```

Five fixture tests passed: person/asset identity distinction, destinations,
shared pages, malformed/mismatched rows/masks, vector/handle limits, and closed
read windows without assuming write eligibility.

## Static native evidence — not runtime attribution

Existing Ghidra project was reused; no whole-program reimport. Targeted exports:
`analysis/native-boarding/lifecycle-followup.txt`, candidates in adjacent JSON.
Executable fingerprint remains `5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.

- Public `api/tealdef/api/engine.d.tl` describes SimPerson destinations as residence,
  shopping and work, and targetOrAtEntity as current or intended location.
  AssetGroup describes scenery assets, particularly trees.
- `0x14256da20`, identified by ApplySimPersonData assertion references, calls
  entity removal `0x142bb75f0` when a state-specific transformation fails.
  Successful branches assert HasHomeAndTarget and SimPersonValidState.
- `0x14256caa0` (ApplySimPersonAtBuildingData) can write the first destination
  (home) to SimPerson+0x50 while applying transformed building data.
- `0x140b2e850` handles target=-1 by setting target to home, removing old journey
  state and using the native return-home path.
- `0x140b326f0` (NotePersonCapacityToBeRemoved) asserts that destination/target
  index sets for a removed capacity are empty; it is not itself the relocation
  algorithm.

These paths establish native lifecycle possibilities. They do not prove which
branch affected these two passengers, whether city development caused it, or
whether the mod influenced the outcome. Do not infer a successful arrival from
missing state, a reused ID, or an unmatched current-home ID.

## Preserved comparison baseline

Restored the verified untouched expanded baseline (`ce2e02d935da…`) as a NEW
save name `TPF3Interlining_before_multi_passenger_01.sav`. No existing save was
replaced. User restarted/loaded this copy paused for original component and
home/target snapshots. Diagnostic checkpoint `acca331ade37…` stays preserved.


## Pre-trial comparison and read-only control

Loaded the restored untouched baseline with no mutation-capable DLL. Fresh
Linux52616 / Windows308, thread628; worlds1 `0x2bd709c0`,2 `0x2bd6fd00`.
Observer `tpf-bridge-308-9872150.log` initially confirms122261 waiting on S3
stops1→2, original target72934. After the user's brief run/pause, repeated entity
inventories show:

| Person | Pre-trial home/shopping/work | Post-trial home/shopping/work | Target change |
| --- | --- | --- | --- |
| 122261 | [122255,75501,72934] | [122255,75501,-1] | 72934→122255 (existing home) |
| 120770 | [87062,-1,72649] | no SimPerson; ID now scenery asset | no arrival proof |

**122261's home did not change; its work destination was cleared.** The post-trial
at-building ID is its original home, not evidence of equivalence with workplace
72934. This comparison alone could not establish the reason or timing of
workplace invalidation; the control below captures its recurrence.

Pre-trial inventory: `lifecycle-v7b/pre-trial-inventory.json`, SHA-256
`6f782eafd17d34fc1db185a1b809849ea8b54f495844b2eaed13d1dd52706c09`.

A bounded external read-only monitor was prepared for both passengers, original
home87062, workplace72934 and home122255. It records only changed component sets
and destination lists/targets, requires repeated read agreement and closed
windows, checks process identity, and has a600-second limit. No interlining
consumer or mutation backend is loaded. Purpose: determine whether both lifecycle
changes occur without line replacements. Source and output are ignored research
artifacts `lifecycle-v7b/watch_control.py` / `control-lifecycle.jsonl`.

## Completed control: both changes occur without interlining

The monitor completed early after **560.06 seconds**, at
**2026-10-08T14:31:42.771544Z**. This duration includes the initial paused wait;
it is not simulation time. It accepted 2,153 world samples and rejected seven
inconsistent reads. Rejected reads were not used as lifecycle evidence.

| Elapsed wall time | Both-world observation |
| --- | --- |
| 0 s | 120770 is a SimPerson with home87062/work72649; 122261 has home122255/work72934. |
| 476.55 s | 120770 has no SimPerson, only an EcsOctreeNode component. Home87062 gains Scaffold. |
| 483.59–483.60 s | Home87062 no longer has Scaffold. Passenger120770 remains absent. |
| 560.06 s | 122261 is at its original home122255 with destinations[122255,75501,-1]. Workplace72934 gains Scaffold and EmissionEmitter. |

The independent in-game read-only observer recorded122261 boarding S3 vehicle97912,
transferring to line79460/vehicle123297, then moving with target72934. Callbacks
2521/2522 report at-building target122255 in worlds1/2. No at-building72934 arrival
was captured. No switching consumer or mutation backend was loaded in this process.

**Passenger removal and work-destination clearing do not require our interlining
mutation.** Both reproduced from the preserved pre-trial save. Their association
with Scaffold changes supports building redevelopment as the explanation, but
the external samples do not establish the exact native call sequence. The control
reused120770's ID for an octree node, whereas the diagnostic checkpoint contained
a scenery asset: exact subsequent ID reuse is not required for this comparison.
This does not rule out every mod defect or establish batch journey completion.

Additional static export `analysis/native-boarding/person-home-transform.txt`
shows `0x140b36140` assigning the first destination to target+0x50; its caller
`0x14256d1d0` invokes this path when the transformed target is -1. This corroborates
native return-home handling, not runtime attribution of the observed event.

Immutable control artifacts in `analysis/native-bridge/lifecycle-v7b/`, SHA-256:

- `control-lifecycle.jsonl`:
  `39e2fcce5940d74131e1db05686074deace9dd7018b3e8b1ff1b92a84a6e4221`.
- `control-observer-at-completion.log` (copy of the still-live observer log):
  `3ccc0e7e1aa81859785043833bf31487395823389b91e5af955eb7d00b909c98`.
- `watch_control.py`:
  `7e3ac4dc28c071cf3ff43811a3587f4d959c4b08a44e1832702dc8ea9ec1b67f`.
- `control-run.json` records the process, baseline, bounds and completion.

The external monitor exited0. The loaded observer may continue read-only until
its 3,600-callback cap and remains loaded until game exit; no more simulation is
needed for this control. User was told the game can be paused or closed.

## Next implementation gate

Add copied passenger identity/destination diagnostics to bounded journey tracking.
Report **original destination reached**, **passenger removed**, **destination
invalidated**, and **unresolved read/identity error** separately. Missing state
alone must never imply successful arrival or automatically permit more mutations.
Preserve the current stop, attempt cap and boarding policy while adding/testing
classification. Only then design a fresh bounded trial with explicit handling
of independently confirmed lifecycle outcomes. The other two original batch
destinations remain unverified; no stable release is justified yet.
