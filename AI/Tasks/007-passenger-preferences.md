# 007 — Add varied journey-sensitive preferences

- Status: Pending
- Owner: Assistant implements; user tests in-game
- Dependencies: 006 correctness established

## Objective and output

Give passengers stable differences in waiting tolerance, comfort preference,
and transfer tolerance. Relate tradeoffs to expected remaining journey duration
so short trips can favor immediate departure and long rides can justify waiting.

## Acceptance criteria

- [ ] Define readable, configurable weights with documented units and defaults.
- [ ] Make preference assignment reproducible and stable across decisions and
      save/reload; avoid repeated random changes that cause indecision.
- [ ] Incorporate waiting and remaining journey estimates, transfers, comfort,
      and crowding only where the inputs are available and validated.
- [ ] Preserve eligibility and journey-correctness rules from 006.
- [ ] Compare short and long trips and differing service quality against the
      bounded prototype; do not assume evenly split ridership is the objective.
- [ ] Measure sensitivity, worst-case waits, journey completion, and runtime cost.
- [ ] Explain sampled decisions through diagnostics and document limitations.

## Validation and open questions

No preference model is implemented. Engine support for relevant measurements
must be established first. Choosing a random line before arrival does not
demonstrate arrival-aware boarding; test actual boarding opportunities.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/007-passenger-preferences.md`.
