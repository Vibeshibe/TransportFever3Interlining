# Candidate policy prototype

`candidate.lua` is a pure Lua module, independent of engine bindings or an
extender. It accepts copied snapshots and produces a proposal or a reason to
retain the current journey. It is not loaded by the probe and cannot modify
passenger state. There is no native adapter yet.

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
