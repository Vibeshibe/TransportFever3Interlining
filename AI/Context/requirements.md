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

The user supplied `TPF3Interlining_baseline`, with S1, S2, and S3 reportedly
sharing three stops before branching, and will provide in-game observations
and testing. A local baseline copy has been preserved. That collaboration is sufficient
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

Current authorization: the user asked to resume implementation at the assistant's
discretion, covering diagnostics and continued native boarding investigation. On 2026-10-07 the user asked to resume
and agreed to test the prepared probe. Task 002 has a first user-observed example and runtime logs; exact boarding
eligibility remains unresolved. No native behavior patch has been implemented or validated.

## Release experience — clarified 2026-10-13

Target a usable mod.io release with faster discovery and fewer delayed/missed
switches. Prefer implementation and targeted regression checks over repeated
long diagnostic trials. The user requires no separately launched loader or other
external software. A shared script-hook dependency enabled alongside the mod is
acceptable if the game can load it normally. Native Linux support matters; Proton
support is not native Linux support. The native Linux installation is
`/home/vibeshibe/.local/share/Steam/steamapps/common/Transport Fever 3/`.

The current0.14.0 research DLL/loader does not meet that release requirement.
A manifest dependency alone cannot supply the missing native loading mechanism.
Resolve this before presenting another native ZIP as a normal mod.io release.
