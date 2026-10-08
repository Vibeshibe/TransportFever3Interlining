# First passenger replacement implementation — 2026-10-08

## Implementation and boundary

Bridge 0.6 / ABI 6 adds a staged `request_waiting_leg_change`, owner-scoped result
lookup, and copied passenger journey state. Existing ABIs retain their layouts.
This is an explicitly loaded experimental native DLL, not an autoloading release.
The pilot consumer requests at most one S1→S3 change for the baseline's adjacent
Wimborne→Melksham leg after the existing full candidate validator accepts it.

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
`0x143cea960`, stride32, line/stops at0/4/8 and vehicle at12). Movement/building flags
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
host behavior; actual native lifecycle/replication still require the live trial.

## Live trial

User restarted, loaded a disposable interlining baseline and left it paused.
GE-Proton11-5 loader identified Windows PID308 and returned zero when loading:
`analysis/native-bridge/build-v6/tpf_shared_bridge.dll`.
DLL SHA-256:
`8867ad17808deaa059c4322e57652205eb5b0b191f740d67943733c6e3293861`.
Log: `analysis/native-bridge/build-v6/tpf-bridge-308-2576496.log`.

Trial pending. Capture is bounded to 3,600 updates and stops early if the target
is observed at its original destination building after boarding in both worlds.
No installed Lua probe, shipped game file or save file was modified by the tools.
The live engine operation, once performed, changes the disposable simulation.
