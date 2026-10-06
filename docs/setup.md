# Workspace setup

## Location

The requested location is `~/Documents/Projects/code/TPF3Interlining`.
This session cannot write there, so the prepared repository is staged at
`/home/vibeshibe/Games/Transport Fever 3/TPF3Interlining`.

After this task finishes, move it from a normal terminal, provided the destination
still does not exist:

```sh
rtk proxy mv -T -n "/home/vibeshibe/Games/Transport Fever 3/TPF3Interlining" "/home/vibeshibe/Documents/Projects/code/TPF3Interlining"
```

The move preserves `.git` and the absolute game symlink. Then open the destination
as the workspace to allow future edits there. No remote has been configured.

## Contributor instructions

Root `AGENTS.md` is an exact copy of the existing game guide. Interpret its game
paths under `game/installation/`; project workflow is described in the README.
Shell commands must be prefixed with `rtk` per `/home/vibeshibe/.codex/RTK.md`.

## Tooling and validation

See [tools/README.md](../tools/README.md) for the inspection command, pinned
Ghidra download/checksum, and optional MCP setup. No tool installation is complete.
To reproduce the initial executable findings:

```sh
rtk proxy python3 tools/inspect_game.py > analysis/executable-report.json
rtk git status --short
rtk git log --oneline
```

The report should match the SHA-256 in the initial research notes. A different
hash means offsets must be re-established for that build.
