# TPF3Interlining

Research and development for passengers using overlapping Transport Fever 3
services, including boarding the next suitable vehicle and varied preferences.
No working game modification exists yet.

## Layout

- `AGENTS.md`: contributor guide copied unchanged from the game installation.
  Its game-relative paths refer to `game/installation/` in this repository.
- `game/installation`: local symlink to the installed game (ignored by Git).
- `docs/`: investigation notes, setup instructions, and progress log.
- `tools/`: reproducible inspection and reverse-engineering helpers.
- `analysis/`: local analysis projects and generated output (ignored by Git).

Start with [the research findings](docs/interlining-research.md) and
[the setup guide](docs/setup.md). Record findings and validation in
[the progress log](docs/progress.md), and commit each coherent milestone.

## Working conventions

Prefix shell commands with `rtk`, as required by
`/home/vibeshibe/.codex/RTK.md`. Use `rtk proxy` for tools without a dedicated
RTK wrapper. Analyze the game through the symlink; put outputs in `analysis/`.
Keep installed binaries, content archives, savegames, and generated databases
out of Git. The symlink points to live files, so treat it as read-only.
