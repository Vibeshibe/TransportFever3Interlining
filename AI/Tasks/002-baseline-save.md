# 002 — Reproduce and record baseline behavior

- Status: In progress — first observation recorded; eligibility details pending
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
- [x] Preserve a baseline and use separate saves for experiments.

## Validation and open questions

The user created `TPF3Interlining_baseline`; its actual filename is
`TPF3Interlining_Baseline.sav`. They report lines S1, S2, and S3 sharing three
stops before branching in different directions. Exact stop names, platforms,
and vehicle settings remain to be recorded; the first observation is below.

A separate local snapshot was copied into ignored
`analysis/baselines/c228e860e25a/` and verified against the original with SHA-256:
`c228e860e25a4cc11804e1b3c750793077b5022ea62b41bf568fc7b2f946389c`.
The source was not modified. See [the handoff](../../docs/test-save.md) for details.
Preservation is complete; use a further working copy for experimental runs.

On 2026-10-07 at 13:52 local, the user observed a passenger continue waiting
for S2 when S1 arrived heading to the same station. [Test 1](../../docs/test-run-2026-10-07.md)
records the working probe, build/mod list, preserved log/save, and evidence
limits. Specific platform access, seats, destination, and passenger identity
remain unknown; the log cannot establish why that vehicle was ignored.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/002-baseline-save.md`.
