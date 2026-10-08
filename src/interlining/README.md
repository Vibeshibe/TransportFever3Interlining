# Candidate policy prototype

`candidate.lua` is a pure Lua module, independent of engine bindings or an
extender. It accepts copied snapshots and produces a proposal or a reason to
retain the current journey. It is not loaded by the probe and cannot modify
passenger state. The native read-only validator below uses the shared bridge SDK.

The first scope is deliberately small: adjacent forward stops, matching boarding
and alighting station/terminal identifiers, an unassigned waiting passenger,
the vehicle at the configured terminal, passenger loading enabled, and observed
unused capacity within the load fraction. Same-line vehicles retain ordinary
engine behavior. Incomplete inputs retain the current journey.

Input examples are in `tests/test_candidate.lua`, based on Test 2's passenger
75625 and S3 vehicle 97959. The route excerpt combines the first snapshot with
later corroborating observations; it is not a complete simultaneous route dump.
Other test cases are synthetic controls, including S2's differing terminal.

```sh
rtk proxy lua tests/test_candidate.lua
```

`status = "candidate"` means only that these policy checks passed. A future
native adapter must revalidate live entities, route revisions, actual access,
loading state, reservations/capacity, and onward-journey consistency at the
engine's mutation boundary. The proposal includes expected and replacement
line/stop/vehicle values for that comparison. Observed unused capacity is not
a seat reservation; tick equality is not proof of freshness when applied later.

Do not apply a proposal by writing fields directly or by merely adding another
line's vehicle to the lookup results. The native trace found separate queue
indexes and explicit line/stop fields copied into onboard state. Migration must
preserve all of them through verified engine transitions. See
[the native trace](../../docs/native-boarding-investigation.md).

Longer segments, loop wrap, alternative terminals, transfers, and varied
preferences remain future work after correctness of the initial case.

## Native read-only validator

`native_candidate.c` uses only the versioned public C API; it contains no engine
offsets or pointers. `it_validate_candidate` re-reads passenger, route, vehicle,
physical platform, terminal permissions, destination presence and loading-index
membership in the current callback. Query failures preserve the caller's output;
ordinary policy exclusions return a named reason. Positive results carry the
expected waiting snapshot, proposed line/stop indices and dispatch sequence.

The native pilot accepts only an adjacent forward leg with identical boarding
and alighting terminals, a locked vehicle in ready Loading state, no alternative
terminals, load mode `LOAD_IF_AVAILABLE`, maximum load exactly 100%, and capacity
after onboard passengers and reservations. Unlike the offline Lua calculation,
it rejects fractional load limits because native compartment allocation and
rounding are unresolved. Empty direct platform membership also rejects.

`requires_engine_revalidation` is always set. Destination presence and matching
alighting terminals do not establish pathfinding success, save compatibility or
safe cross-line mutation. ABI 5 provides read-only queries. The research observer
in `native/candidate_probe.c` samples the baseline save and logs proposals;
repeated proposals are not unique passengers or completed boardings.

Build and run `candidate_tests.exe` using the [native instructions](../../native/README.md).

## One-passenger native trial

Bridge 0.6.1 uses the validator in a behavior-changing consumer: the first eligible
baseline S1↔S3 adjacent Wimborne→Melksham proposal is submitted through ABI 6.
The host stages it until observers finish, invokes the consumer's full validator
again, compares expected state and applies engine-owned remove/add in a dedicated
scope. The host allows one engine attempt across all clients/worlds per load.
The consumer then tracks boarding and destination state; it does not switch again.

This remains an explicit development loader, with no persistent install or Lua
binding. See [the trial record](../../docs/native-mutation-validation-2026-10-08.md).
