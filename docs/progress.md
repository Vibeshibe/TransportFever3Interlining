# Progress log

## 2026-10-06 — Repository foundation

- Copied the installation's `AGENTS.md` unchanged and imported the initial
  interlining feasibility assessment.
- Added documentation, tooling, game-reference, and local-analysis directories.
- Created an ignored absolute symlink to the game installation.
- Requested destination: `~/Documents/Projects/code/TPF3Interlining`.
- Sandbox denied creating that directory with `Read-only file system`.
  Prepared this repository under the writable game directory for relocation.
- Git 2.55.0, Python 3.14.7, and OpenJDK 21 are available.
- Direct access to the GitHub API failed with `Could not resolve host`.
  Tool installation is not complete; browser research remains available.

## 2026-10-06 — Reproducible inspection tools

- Foundation committed as `f70618a` (`Initialize interlining research workspace`).
- Added `tools/inspect_game.py` to reproduce the executable fingerprint and
  seven diagnostic string anchors, including PE section-to-address mapping.
- Validation passed against all seven previously recorded offsets and the
  known SHA-256. Invalid and missing input files return clear errors.
- Confirmed the copied `AGENTS.md` is byte-for-byte identical to the original,
  the game symlink resolves correctly, and Git ignores game files, reports,
  downloaded tools, and Python environments.
- Generated `analysis/executable-report.json` locally; it is excluded from Git.
- Recorded official Ghidra 12.1.3 download instructions and its published
  SHA-256. Its documented minimum JDK is 21, matching the default local JDK.
- Attempting the actual release download also failed with
  `Could not resolve host: github.com`; Ghidra remains uninstalled.
- Documented optional GhidraMCP setup and the need to verify compatibility.
  No MCP extension, bridge dependencies, or client registration is installed.
- No native code has been decompiled and no game patch or runtime validation
  has been performed. Next: relocate the workspace, finish Ghidra installation
  with network access, then follow cross-references to `OnModifyPathfinding`
  and the vehicle-arrival / passenger-queue diagnostic strings.

## Ongoing workflow

Commit coherent research, tooling, and implementation milestones separately.
For each milestone record evidence, commands, outcomes, unresolved assumptions,
and the next validation step. Label static inference separately from runtime
verification. Do not claim installation or gameplay validation without running it.
