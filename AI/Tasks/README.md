# Tasks

Track actionable work here. Each task should state its objective, status,
acceptance criteria, validation results, and related commits. Keep durable
technical findings in `AI/Context/` or `docs/` and link to them.

## Task board

| ID | Task | Status | Owner | Dependencies |
| --- | --- | --- | --- | --- |
| [001](001-project-plan.md) | Establish plan and requirements | Complete | Assistant | None |
| [002](002-baseline-save.md) | Reproduce and record baseline behavior | In progress: save being built | User; assistant supports observations | None |
| [003](003-scripting-hook.md) | Establish scripting-hook capabilities | Ready; not started | Assistant | 001; runtime probe uses 002 |
| [004](004-native-boarding.md) | Trace native boarding and state transitions | Conditional; not started | Assistant | 003 identifies an API gap |
| [005](005-shared-extender.md) | Design a reusable native extension, if needed | Conditional; not started | Assistant | 004 verifies an intervention point |
| [006](006-interlining-prototype.md) | Implement and validate alternative boarding | Pending | Assistant; user tests | 002, 003; 004–005 if needed |
| [007](007-passenger-preferences.md) | Add varied journey-sensitive preferences | Pending | Assistant; user tests | 006 correctness established |
| [008](008-stable-release.md) | Validate and promote a stable milestone | Pending | Assistant; user tests | 006; 007 if included in release |

The user is preparing task 002. Static work in 003 can proceed independently
once authorized; lack of a save does not block code inspection. Tasks 004–005
are optional and must be marked skipped with evidence if scripting suffices.

## Tracking conventions

Use `Ready`, `In progress`, `Pending`, `Blocked`, `Complete`, or `Skipped`, with
a short reason where useful. Update this board and the task record together.
Completion requires evidence against the acceptance criteria. Separate source
inspection, static inference, runtime observations, and proposals explicitly.

For each milestone, add validation and remaining questions to the task and
`docs/progress.md`, then commit and push on `dev`. Record relevant commit hashes
when known; use `git log -- <task-path>` to find the task's planning history.
Do not treat a proposed API or a successful tool launch as a working game fix.

See [research notes](../../docs/interlining-research.md) for investigation targets
and [the progress log](../../docs/progress.md) for completed work.
