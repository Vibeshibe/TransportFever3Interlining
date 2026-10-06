# TPF3Interlining

Research and development for passengers using overlapping Transport Fever 3
services, including boarding the next suitable vehicle and varied preferences.
No working game modification exists yet.

## Layout

- `AGENTS.md`: game contributor guide with project-specific branch instructions.
  Its game-relative paths refer to `game/installation/` in this repository.
- `game/installation`: local symlink to the installed game (ignored by Git).
- `docs/`: investigation notes, setup instructions, and progress log.
- `tools/`: reproducible inspection and reverse-engineering helpers.
- `analysis/`: local analysis projects and generated output (ignored by Git).
- `AI/Tasks/`: actionable work and acceptance criteria.
- `AI/Context/`: project orientation and durable handoff notes.

Start with [the research findings](docs/interlining-research.md) and
[the setup guide](docs/setup.md). Record findings and validation in
[the progress log](docs/progress.md), and commit each coherent milestone.
The [task board](AI/Tasks/README.md) tracks ownership and acceptance criteria;
the [test-save guide](docs/test-save.md) describes the baseline handoff.

## Working conventions

Prefix shell commands with `rtk`, as required by
`/home/vibeshibe/.codex/RTK.md`. Use `rtk proxy` for tools without a dedicated
RTK wrapper. Analyze the game through the symlink; put outputs in `analysis/`.
Keep installed binaries, content archives, savegames, and generated databases
out of Git. The symlink points to live files, so treat it as read-only.

## Branch workflow

Develop and commit research, tooling, and implementation on `dev`; push coherent
milestones to the private `origin/dev`. Use `main` for validated stable releases.
Before promoting a milestone, review the changes and document the relevant
checks, including in-game validation for behavior changes. Pushes are authorized
by the repository owner. See [setup status](docs/setup.md) for current blockers.
