# 002 — Reproduce and record baseline behavior

- Status: In progress — user is building the sandbox save
- Owner: User builds and runs the save; assistant defines observations and diagnostics
- Dependencies: None; diagnostics may depend on task 003

## Objective and output

Establish a repeatable case of passengers ignoring usable overlapping services.
Keep an untouched baseline save and record its local location/version in the
handoff. Use [the test-save guide](../../docs/test-save.md); save binaries stay
outside tracked files.

## Acceptance criteria

- [ ] Three lines share a trunk and branch, with identifiable stops and directions.
- [ ] Initial vehicles are identical, have spare seats, and permit passenger loading.
- [ ] Shared-section and branch-destination trips both exist and are observable.
- [ ] Record game build, active mods, simulation speed, and relevant line settings.
- [ ] Document repeatable examples with destination, ignored/boarded service,
      available capacity, and observation time or sequence.
- [ ] Distinguish a planned transfer from the passenger's final destination
      wherever the available information permits.
- [ ] Preserve a baseline and use separate saves for experiments.

## Validation and open questions

No baseline observations have been supplied yet. User observations are adequate
to start; instrumentation should supplement them where the visible UI cannot
explain a decision. Determine which stop/platform and route details are visible
before requesting extra data. Measure repeated cases rather than promising an
effect from a single boarding event.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/002-baseline-save.md`.
