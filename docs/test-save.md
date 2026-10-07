# Baseline test-save handoff

The user supplied this save for [task 002](../AI/Tasks/002-baseline-save.md).
The assistant will use observations and, when possible, diagnostics to compare
baseline and modified behavior. Results are recorded in [Test 1](test-run-2026-10-07.md) and
[Test 2](test-run-2026-10-07-2.md), which captures a same-terminal reference case.

## Received baseline

- User's save label: `TPF3Interlining_baseline`.
- Actual filename: `TPF3Interlining_Baseline.sav`.
- Reported topology: S1, S2, and S3 share three stops, then split in different
  directions. This is user-reported; no in-game inspection has been performed.
- Preserved local copy:
  `analysis/baselines/c228e860e25a/TPF3Interlining_Baseline.sav`.
- Size: 76,126,628 bytes.
- SHA-256: `c228e860e25a4cc11804e1b3c750793077b5022ea62b41bf568fc7b2f946389c`.
- Local `manifest.json` alongside the copy records the original save path,
  source modification time, hash, and handoff details. Both files are Git-ignored.

The snapshot hash matches the source, which was not modified. Keep this snapshot
as the baseline and use a separate working copy for experiments. Game build,
mods, stop/platform details, vehicle settings, and boarding examples still need
to be recorded; finding the file does not validate its in-game behavior.

## Initial scenario

Create three lines sharing several stops, then branching like the supplied
diagram. Label stops and directions clearly. Use identical vehicles initially,
with spare seats and passenger loading enabled. Where possible, use the same
boarding location on the trunk; record any platform differences.

Include demand for both trips entirely within the shared section and trips
continuing beyond each branch. Preserve an untouched baseline save and use
separate copies for experiments. Save files can remain in the normal save
directory or ignored `analysis/`; provide a local path when available.

## Handoff details

- Save label/local path and game build/version.
- Active mods and relevant settings, including simulation speed.
- Line names, ordered stops, platforms, vehicle type/count, and departure rules.
- A reproducible observation point or sequence after loading the save.
- Any UI or logging limits that prevent identifying a passenger's next alighting
  stop versus their final destination.

## Observation template

Copy this table into a test report when results are available. Unknown fields
can remain unknown; screenshots or short recordings can supplement descriptions.

| Field | Observation |
| --- | --- |
| Save/build/mod version | |
| Baseline or modified run | |
| Observation time/sequence and simulation speed | |
| Passenger identifier, if available | |
| Waiting stop/platform and travel direction | |
| Destination / planned alighting stop, if known | |
| Arriving line and available passenger seats | |
| Boarded or ignored; subsequent service boarded | |
| Approximate wait and trip completion | |
| Evidence, uncertainty, or other conditions | |

Record multiple opportunities under comparable conditions. Stop layouts, walk
access, capacity, destination, and load rules can explain an ignored vehicle;
keep those details alongside the observation rather than assuming the cause.

## Later comparison cases

After the basic case is established, vary capacity, vehicle quality, frequency,
trip length, platform access, and express versus local stopping patterns one at
a time. Check branch destinations, transfers, line edits/removal, and save/reload.
Report journey completion and waiting/travel time as well as passenger counts;
spreading riders among lines alone does not establish an improvement.
