# Candidate policy prototype

`candidate.lua` is a pure Lua module, independent of engine bindings or an
extender. It accepts copied snapshots and produces a proposal or a reason to
retain the current journey. It is not loaded by the probe and cannot modify
passenger state. The native read-only validator below uses the shared bridge SDK.

The historical Lua scope is deliberately small: adjacent forward stops, matching boarding
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

The native implementation below now expands shared segments; this historical Lua
prototype still tests adjacent legs. Loop wrap, alternative terminals, planning
new transfers and varied preferences remain outside the current native policy.

## Native read-only validator

`native_candidate.c` uses only the versioned public C API; it contains no engine
offsets or pointers. `it_validate_candidate` re-reads passenger, route, vehicle,
physical platform, terminal permissions, destination presence and loading-index
membership in the current callback. Query failures preserve the caller's output;
ordinary policy exclusions return a named reason. Positive results carry the
expected waiting snapshot, proposed line/stop indices and dispatch sequence.

Version 0.8 accepts forward legs of one to four stop intervals. The substitute
must follow an ordered subset of the original station groups through its original
alighting stop: express services may skip intermediate stops, but extra stops,
repeated stations, branch detours and loop wrapping are excluded. Identical boarding
and alighting terminals are required, along with a locked vehicle in ready Loading
state, no alternative endpoint terminals, load mode `LOAD_IF_AVAILABLE`, maximum load exactly 100%, and capacity
after onboard passengers and reservations. Unlike the offline Lua calculation,
it rejects fractional load limits because native compartment allocation and
rounding are unresolved. Empty direct platform membership also rejects.

`requires_engine_revalidation` is always set. Destination presence and matching
alighting terminals do not establish pathfinding success, save compatibility or
safe cross-line mutation. ABI 5 provides read-only queries. The consumer
in `native/candidate_probe.c` discovers demand lines and logs proposals;
repeated proposals are not unique passengers or completed boardings.

Build and run `candidate_tests.exe` using the [native instructions](../../native/README.md).

## Shared leg versus final destination

Eligibility preserves the passenger's existing planned alighting stop. The
four-interval bound applies to that train leg; the final destination may
require further travel. For example, an S1 leg from Wimborne to Melksham can be
replaced by S3 over those same stops even when the game then continues on S2.
The mod does not choose a new interchange, shorten a through leg, or deliberately
send a passenger ahead to wait for their original train at another station.

The [reload trial](../../docs/save-reload-trial.md) observed this native onward
transfer after a saved S1→S3 switch. The complete original itinerary was not
captured, so the pre-switch identity of the next connection is unverified.
The pilot checks route compatibility; it does not yet compare downstream waits,
whole-trip duration, fare or comfort. Earlier boarding alone is not evidence of
earlier arrival. Broader preference and timing policy requires separate work.

## Bounded native trials

Bridge 0.6.4 used the validator in a behavior-changing consumer: the first eligible
baseline S1↔S3 adjacent Wimborne→Melksham proposal is submitted through ABI 6.
The host stages it until observers finish, invokes the consumer's full validator
again, compares expected state and applies engine-owned remove/add in a dedicated
scope. That revision allowed one engine attempt across all clients/worlds per load.

Version 0.7 retains these eligibility rules and supports an explicit 1–5 passenger
build cap, default one. The consumer waits for the expected train/leg boarding in
both worlds before selecting the next person, never selects an already attempted
person, and tracks each original destination through native onward travel.
The host separately enforces distinct-person and per-dispatch bounds and stops
future writes after failed postconditions. Capacity is queried again for every
proposal and revalidation; discovery and tracking use separate callback budgets.
See [the multi-passenger trial](../../docs/multi-passenger-trial.md) for current
validation evidence.

This remains an explicit development loader, with no persistent install or Lua
binding. See [the trial record](../../docs/native-mutation-validation-2026-10-08.md).

## Version 0.8 route expansion

The save-specific line/stop gate is removed. Discovery learns lines from observed
waiting passengers with bounded rolling caches and independent stop cursors.
Candidate routes are copied and compared again before the deferred change;
route edits invalidate the proposal. Four passenger screens keep the worst
fixture at 62/64 queries without changing the shared SDK or mutation operation.
The passenger cap, same-platform/capacity checks and lifecycle handling remain.

Multi-stop/express, dynamic discovery, stale data and route-edit regressions pass.
Build-v8a completed one adjacent journey before stopping on unsupported waiting
sentinels; multi-stop/express live coverage remains pending. Version0.8.1
preserves known -2/-3 values for discovery and excludes them from switching.
Its completed capture confirmed three adjacent boardings and two original
arrivals, with the third journey unfinished at the callback cap. Zero query
errors; the special sentinel paths did not appear in this live run. See
[scope, examples and trial instructions](../../docs/route-expansion.md).
