# Context

Store concise handoff notes, confirmed constraints, and architecture decisions
here. Include evidence and distinguish static inference from runtime findings.
Link to existing documentation instead of duplicating detailed research.

## Project orientation

- Goal: let passengers use suitable overlapping services, with varied preferences
  and journey-duration-sensitive tradeoffs between waiting and comfort.
- Repository: `/home/vibeshibe/Documents/Projects/code/TPF3Interlining`.
- Game reference: `game/installation`, an ignored symlink to the live game.
- Initial assessment: [interlining research](../../docs/interlining-research.md).
- Setup and tool status: [workspace setup](../../docs/setup.md) and
  [research tools](../../tools/README.md).
- Work history: [progress log](../../docs/progress.md).
- Agreed behavior, collaboration, and extender requirements:
  [requirements](requirements.md).
- Active work and dependencies: [task board](../Tasks/README.md). Step 1 is the
  planning milestone; the user is building the baseline save for task 002.
- Branch policy: research and implementation on `dev`, validated stable releases
  on `main`. The user authorizes regular commits and pushes to private `origin`.
  `dev` tracks `origin/dev`. Permission-enabled commands can write Git metadata
  and authenticate to the server through the configured `libsecret` helper.
- No working patch or in-game validation exists yet. Commit coherent milestones
  and record their evidence and remaining uncertainties.
