# First passenger replacement implementation — 2026-10-08

## Implementation and boundary

Bridge 0.6 / ABI 6 adds a staged `request_waiting_leg_change`, owner-scoped result
lookup, and copied passenger journey state. Existing ABIs retain their layouts.
This is an explicitly loaded experimental native DLL, not an autoloading release.
The initial pilot permits one S1→S3 change; revision 0.6.1 permits either S1/S3
direction on the baseline's adjacent Wimborne→Melksham leg. The existing full
candidate validator must accept it before staging.

The generic host copies the request and complete native waiting component,
including padding. It waits until all registered observers finish, rejects all
competing requests for the same passenger, and calls the requesting consumer's
revalidator with a fresh callback token. The consumer re-runs current route,
platform, loading, destination and capacity checks. The backend independently
compares the expected waiting/destination/place snapshot and all 32 native waiting
bytes, and requires the passenger to be in waiting state alone.

The backend opens an engine modification scope, removes and re-adds the waiting
component with notifications, then closes the scope before original terminal
selection. Only line and boarding/alighting indices change. Original arrival time,
unassigned vehicle, destination and place are checked after the operation. A
process-wide allowance permits only one engine attempt, even across different
worlds or clients and even when a postcondition fails. No automatic retry or
rollback is claimed after an engine call begins.

Native clients remain trusted code: revalidation callbacks must return normally,
and the host cannot contain arbitrary native exceptions, asserts or hangs. The
change service is not a route planner; the interlining consumer owns eligibility
checks. Results distinguish staging, conflict, changed expected state, rejection
and applied replacement. Applied does not mean a passenger has boarded.

## Binding provenance

Pinned executable SHA-256:
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
New callables are checked against 16 live entry bytes in addition to the whole
file hash and existing slot/table verification. Addresses use preferred base
`0x140000000`; runtime addresses are relocated.

| Operation | Preferred VA | Windows x64 arguments |
| --- | --- | --- |
| Begin modification | `0x142bb4640` | world |
| Remove waiting component | `0x142bb7320` | world, person, RTTI `0x143cea8f0`, notify=1 |
| Add waiting component | `0x140b21cd0` | world, person, copied 32-byte value, notify=1, local storage=1 |
| End modification | `0x142bb4d90` | world |

The fifth add argument is explicitly verified at the native same-line remapping
call site: `0x140b3116d` writes byte 1 to stack `+0x20`, preceding the call at
`0x140b3117e`. Ghidra's four-argument caller rendering omits this stack argument.
The callee uses it to distinguish contiguous and shared storage.

Removal and addition dispatch terminal/person lifecycle notifications, maintaining
queue, station and line-person indexes. Begin/end and component operations enter
the native replication stream. `0x140255de0` passes source world `+0x18` and target
world `+0x18` through `0x142bb78f0`, which replays the recorded operations.
The host never manually applies the same replacement to a second world.

New ignored exports: `mutation-bindings.txt`, `mutation-world-dispatch.txt` under
`analysis/native-boarding/`. Existing `queue-migration.txt`, `engine-modification.txt`
and `line-change-transitions.txt` supply the original operation evidence.

Journey observation uses verified waiting/onboard layouts (onboard RTTI
`0x143cea960`, stride 32, line/stops at 0/4/8 and vehicle at 12). Movement/building flags
come from component masks using RTTI `0x143ceaa00` and `0x143ceaa68`. The latter's
layout is not guessed or dereferenced. The reader exposes an opaque process-local
world ordinal; native world pointers stay private. `targetOrAtEntity` is read
from Person `+0x50`. Building presence with the original target after an observed
onboard state is the pilot's destination-arrival observation, not a general
pathfinding or transfer validator.

## Offline validation

Built `analysis/native-bridge/build-v6/` using the pinned LLVM-MinGW toolchain,
with `-Wall -Wextra -Werror`. All three suites passed under the isolated Wine
prefix: `bridge_tests.exe`, `query_tests.exe`, `candidate_tests.exe`.

New fixtures cover deferred writes, conflicts between two clients, result
ownership, old/new context tokens, wrong threads, policy rejection, changed
padding/destination, existing modification windows, insufficient query budget,
exact native call order/fifth argument, preservation of all bytes outside the
three changed integers, postcondition failure and the lifetime attempt limit.
Journey fixtures cover waiting, onboard (including shared pages), moving,
at-building and contradictory waiting/onboard masks. Mock operations establish
host behavior; live lifecycle and replication evidence is recorded below.

## Live trial

User restarted, loaded a disposable interlining baseline and left it paused.
GE-Proton11-5 loader identified Windows PID 308 and returned zero when loading:
`analysis/native-bridge/build-v6/tpf_shared_bridge.dll`.
DLL SHA-256:
`8867ad17808deaa059c4322e57652205eb5b0b191f740d67943733c6e3293861`.
Log: `analysis/native-bridge/build-v6/tpf-bridge-308-2576496.log`.

The first run was ended by the requested game restart before its automatic cap.
It recorded 34 changed candidate results, including two positive S3→S1 records,
but no staged request or engine attempt. No query error was logged. The last
changed-result record is sequence 3327; this is not a final callback count.
First-run log SHA-256:
`e42ddafe5c2773223155bdf7c0f650f0aa6683bee54e3ce7ac41ee55ccb850e0`.

Capture is bounded to 3,600 updates and stops early if the target is observed at
its original destination building after boarding in both worlds.
No installed Lua probe, shipped game file or save file was modified by the tools.
The live engine operation, once performed, changes the disposable simulation.

## Follow-up selection revision

The first trial repeatedly observed a valid S3→S1 adjacent candidate (person
114813 at sequences 421 and 2045), excluded only by the pilot's directional filter.
Version 0.6.1 permits S1↔S3 on the same adjacent Wimborne→Melksham segment.
The full validator, mutation backend, request guards and one-attempt allowance
are unchanged. This is a research-consumer selection change, not broader route
support. It is built separately as `build-v6b`; the loaded `build-v6` is preserved.

Revision build SHA-256:
`81acaeacbaacaa59842bb01aac5000fcfd77e11f78c19b2b4a60fe31ce324f70`.
It compiles with warnings as errors. The native operation, host and query code
are unchanged from the passing test suites; only the research selection condition
and version banner changed. User was asked to restart the same disposable baseline
and retain the platform layout. S2's distinct platform remains a negative control.

The user restarted and loaded the disposable baseline paused. Revision 0.6.1
loaded into Windows PID 312 with `tfx_start` returning zero. New capture:
`analysis/native-bridge/build-v6b/tpf-bridge-312-3397243.log`.
The user unpaused at normal speed. At sequence **2473**, person **72845**
qualified on S1 (`98278`) stops **1→2** for S3 (`97743`) vehicle **97914**,
also stops **1→2**, with **39** places remaining and destination **72405**.
Boarding station **97948** terminal **1** and alighting station **97294** terminal
**0** matched. Ticket 1 was staged, revalidated and applied in world `0x32cd01e0`.
The backend logged preserved arrival/destination/place and a closed modification
scope. Arrival-time bits were `0x0000000000461ba8`.

The following callbacks observed onboard state (`flags=2`) on S3 vehicle 97914
with destination 72405 and stops 1→2 in **both** engine worlds (world IDs 2 then 1).
Only one `change_begin` was logged. This establishes an actual engine-owned
cross-line replacement and subsequent boarding, with propagation to both observed
worlds. The main capture then observed departure from the vehicle in both worlds;
vehicle queries at each transition reported **S3 stop 2**, matching planned
alighting. The passenger entered onward movement (`flags=4`), destination 72405
unchanged. The capture reached 3,600 accepted callbacks and auto-disabled.

A separate **read-only external snapshot** then found at-building state (`flags=8`)
with target/at-entity **72405** in both worlds. Thus this one switched passenger
was observed boarding the alternative line, alighting at the intended stop and
reaching the original destination. The follow-up did not re-enable the bridge or
make another engine change.

## Discovery maintenance prepared during the trial

Code review found that the observer kept its first 32 person IDs indefinitely,
which could miss later arrivals after the slots filled. Revision 0.6.2 replaces
older discovery IDs as newly scanned people arrive, clears their log-suppression
state, and still re-reads eligibility in the current callback. It adds no engine
write capability and keeps the same single-attempt limit. A regression exercises
96 successive IDs within 32 slots, admitting later people without duplicates or
churn when an existing ID is seen again; it and the candidate policy suite pass.

Built separately at `analysis/native-bridge/build-v6c/`, DLL SHA-256
`bce9aedbc8b3685dc7497ec5ab9ab5b89642a994c52c1bc8bb5afcf3ca863fe1`.
This revision has **not** been loaded; the successful live switch above used
revision 0.6.1 (`build-v6b`). Both loaded artifacts remain untouched.

## Final result and artifact hashes

The injected capture completed 3,600 callbacks with **one** staged request,
**one** engine attempt, **one** applied replacement, 31 changed candidate records,
six journey-state records and two alighting checks. Zero query errors or rejected
boundaries were logged. Simulation thread 592; world 1 `0x32cd01e0`, world 2
`0x32ccf520`. Both preserved the same person, destination and new onboard line.
The hook now forwards only. User was told they can pause or close the game.

| Artifact | SHA-256 |
| --- | --- |
| `build-v6b/tpf-bridge-312-3397243.log` | `bcae08aab4ce0e2e9da1084b7b95ccc68f1cd785c6e109464897c8aaed228b9a` |
| `build-v6b/journey-followup-72845.log` | `bb87c9ecd99212aa50c6011baad4215e19921448142ae43f51ba7f2dc84302b4` |
| `build-v6b/journey_snapshot-captured.exe` | `f04cddb61c638852883b8a4d975a386cf6c5e56e45a4bb911ccbd3a2d73a012b` |

Paths above are relative to ignored `analysis/native-bridge/`. The standalone
snapshot reader uses the existing bounded queries with `PROCESS_VM_READ`, finds
the game's module base and accepts explicitly supplied worlds from this capture.
It requires matching successive reads and an empty modification window before
and after. The snapshots are observations, not an atomic cross-world transaction;
the game was running. Two accepted samples reported original-destination building
state, one per world, and the reader exited successfully. It uses no injection,
remote thread or write permission. The exact manually compiled helper is preserved
above; a standard CMake target is now provided for future use.

## Remaining validation

This establishes one complete alternate-line journey on the pinned build and
disposable baseline. It does not establish long-run stability, a full passenger
census, arbitrary save/reload compatibility, line-edit behavior or multi-mod
mutation coexistence in the game. The prepared 0.6.2 discovery change has regression
tests but has not been exercised live. Next repeat controlled journeys and test
save/reload/disable behavior before enabling multiple passengers. No platform
walking, longer shared legs, express-service planning or preference model was added.
