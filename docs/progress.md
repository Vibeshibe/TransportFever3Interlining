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

## 2026-10-06 — Relocated workspace and supplied Ghidra

- Confirmed the active project is now
  `/home/vibeshibe/Documents/Projects/code/TPF3Interlining` and the game symlink
  still resolves to `/home/vibeshibe/Games/Transport Fever 3`.
- Added `AI/Tasks/` and `AI/Context/`, each with a tracked README describing
  its purpose and linking to existing research and progress notes.
- Found the user-supplied extracted distribution at
  `tools/downloads/ghidra_12.1.3_PUBLIC`. Local properties identify version
  12.1.3 PUBLIC, revision `8b4c91d4d5bd1549622bfbade0df199585b98365`.
- Added `tools/ghidra_headless.py` to launch that copy with project-local
  settings/cache. Ran `rtk proxy python3 tools/ghidra_headless.py`: Java 21
  started Ghidra and displayed analyzer usage (expected status 1 without project
  arguments). This validates startup, not binary import or decompilation.
- No archive checksum verification was possible because the original ZIP was
  not supplied. MCP is still optional and uninstalled.
- Updated setup instructions to remove the completed relocation step.
- Validation: AI directory files exist, the game executable is reachable, the
  launcher parses successfully, and `git diff --check` passed.
- Commit blocked: this session mounts `.git` read-only. `git add` failed while
  creating `.git/index.lock` with `Read-only file system`. Changes are saved in
  the working tree but remain unstaged and uncommitted.

## 2026-10-06 — Private origin and branch policy

- User configured the private `origin` and authorized pushes during development.
- Adopted `dev` for development and `main` for validated stable releases, with
  the policy recorded in `AGENTS.md`, README, and AI context.
- Inspected local refs: `main` and cached `origin/main` point to `734a10e`.
- `rtk git switch -c dev` failed creating `.git/refs/heads/dev.lock` because
  `.git` is mounted read-only. The active branch remains `main`.
- A noninteractive `git ls-remote --heads origin` check failed with
  `Could not resolve host: git.local.shibabox.eu`. Remote state was not verified.
- No new branch, commit, or push completed. Pending changes remain in the
  working tree; setup documentation includes the commands to finish the work.

## 2026-10-06 — Sandbox escalation available; authentication pending

- Session permissions now allow reviewed execution outside the sandbox.
- Successfully created and switched to `dev` from `734a10e` using that route.
- The remote check now reaches the HTTPS Git server. It fails with
  `could not read Username ... terminal prompts disabled`, replacing the earlier
  DNS failure. No Git credential helper is configured in the current environment.
- Updated setup and context documents to reflect the resolved sandbox blockers
  and the remaining authentication requirement. Prepared the AI directories,
  Ghidra launcher, and workflow documentation for a development commit.
- Existing Ghidra startup and executable-inspection checks remain applicable;
  this milestone makes no changes to game behavior.

## Ongoing workflow

Commit coherent research, tooling, and implementation milestones separately.
For each milestone record evidence, commands, outcomes, unresolved assumptions,
and the next validation step. Label static inference separately from runtime
verification. Do not claim installation or gameplay validation without running it.
