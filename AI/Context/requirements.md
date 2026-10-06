# Agreed requirements and collaboration

## Passenger behavior

The user wants lines sharing stops before branching to be usable as alternative
services. Passengers should consider boarding the next suitable arriving vehicle
and have varied preferences, including journey-dependent tradeoffs between
waiting, comfort, and travel time. Preserve valid destinations and transfers.
Neither uniform distribution among lines nor random assignment alone is a
success criterion.

The first proposed prototype is bounded to alternative services reaching the
passenger's existing planned alighting stop. More general replanning can follow
after correctness is established. This is a plan, not proven engine behavior.

## Collaboration

The user is building a sandbox save reproducing the three-line shared-trunk
example and will provide in-game observations and testing. That is sufficient
for early runtime collaboration. The assistant handles static inspection,
diagnostics, implementation, and test procedures. Add instrumentation where UI
observations leave important questions unanswered.

## Extension requirements

Investigate the existing scripting hook first. If an extender is necessary,
the user requires a reusable, extensible, human-readable foundation that reduces
conflicts among cooperating mods. Separate engine capabilities from interlining
policy; document API versions, examples, subscription ordering, conflict handling,
errors, lifecycle, and game-version compatibility. Centralize native hooks and
provide safe fallback for unsupported cases.

Do not build an independent bespoke engine patch into every consumer. Equally,
do not claim compatibility with unrelated tools modifying the same functions
without evidence. The actual extension scope follows the capability findings.

## Evidence and delivery

Record observed facts, static inferences, and proposals separately. Existing
research is in [interlining-research.md](../../docs/interlining-research.md).
Work on `dev`, commit and push coherent milestones, and reserve `main` for
validated stable releases. Game files and saves remain outside tracked files.

Current authorization for this milestone: establish the task plan (001).
No new decompilation or behavior implementation is part of step 1. Task 003 is
ready for a subsequent investigation; the user is independently working on 002.
