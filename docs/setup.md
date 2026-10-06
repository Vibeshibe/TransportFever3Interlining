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
