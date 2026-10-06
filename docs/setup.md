# Workspace setup

## Location

The repository has been relocated to
`/home/vibeshibe/Documents/Projects/code/TPF3Interlining` and is now the active
workspace. Its absolute `game/installation` symlink still resolves to the game.
The user configured private remote `origin` at
`https://git.local.shibabox.eu/vibeshibe/TPF3Interlining.git`.

## Contributor instructions

Root `AGENTS.md` was copied from the game guide and now includes this project's
branch and delivery policy. Interpret its game paths under `game/installation/`;
project workflow is described in the README.
Shell commands must be prefixed with `rtk` per `/home/vibeshibe/.codex/RTK.md`.

## Tooling and validation

See [tools/README.md](../tools/README.md) for the inspection command, pinned
Ghidra setup, and optional MCP integration. The user supplied an extracted
Ghidra 12.1.3 distribution in `tools/downloads/ghidra_12.1.3_PUBLIC`.
To reproduce the initial executable findings:

```sh
rtk proxy python3 tools/inspect_game.py > analysis/executable-report.json
rtk git status --short
rtk git log --oneline
```

The report should match the SHA-256 in the initial research notes. A different
hash means offsets must be re-established for that build.

## Branch setup status

Policy: development on `dev`, validated stable releases on `main`. The user has
authorized ongoing commits and pushes to `origin`.

The active development branch is `dev`, created from `main` at `734a10e`.
Permission-enabled execution resolved the sandbox's Git-write and network
restrictions. The HTTPS remote is reachable. This repository now uses the
already-installed `libsecret` credential helper. The user authenticated through
their desktop terminal, and a subsequent noninteractive remote query succeeded.
`dev` tracks `origin/dev`; routine development commits can now be pushed.

Publish development commits with:

```sh
rtk git push -u origin dev
```

If credentials expire, run the push from a regular desktop terminal and enter
the Git server username and password or access token when prompted. Unlock the
desktop keyring if asked.
Do not put credentials in repository files, remote URLs, or the chat.
