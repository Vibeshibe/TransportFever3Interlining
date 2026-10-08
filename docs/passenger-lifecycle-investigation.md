# Passenger lifecycle investigation — 2026-10-08

## Questions and current evidence

The four-passenger trial ended on missing journey state for ID120770. A second
passenger,122261, later had at-building target122255 rather than original72934.
The trial's original-destination completion remains unproven. No further
passenger mutation is authorized by any of the observations below.

A read-only restart into the diagnostic checkpoint confirmed:

- **120770 is now a scenery asset ID**, with ModelInstanceList, BoundingVolume,
  AssetGroup, AssetGroupAutoRemove, EmissionEmitter and OctreeSystemNode components.
  No SimPerson or journey components remain. Numeric entity presence alone cannot
  establish continued passenger identity. The removal/reuse cause is not captured.
- **122261 remains a SimPerson**, with SimEntityAtBuilding. Its current target is
  **122255**, matching the first (home) entry in destinations **[122255,75501,-1]**.
  Both72934 and122255 currently have TownBuilding and PersonCapacity components.
  This does not establish that the original72934 target was visited, nor that the
  two IDs denote the same building. A pre-trial comparison is pending.
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

## Next comparison

Restored the verified untouched expanded baseline (`ce2e02d935da…`) as a NEW
save name `TPF3Interlining_before_multi_passenger_01.sav`. No existing save was
replaced. User will restart/load this copy paused for original component and
home/target snapshots. Diagnostic checkpoint `acca331ade37…` stays preserved.
