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

## 2026-10-06 — Desktop credential helper configured

- Committed the pending workspace setup on `dev` as `8187d97`.
- Detected `/usr/lib/git-core/git-credential-libsecret` and configured it for
  this repository with `git config --local credential.helper libsecret`.
- Retried the remote query through permission-enabled execution. Credentials
  remain unavailable: Git reports that it cannot read a username with terminal
  prompts disabled. The user must authenticate once in their desktop terminal.
- No password or token was requested through chat or stored in repository files.

## 2026-10-06 — Authenticated remote verified

- User completed terminal authentication and pushed `dev`.
- A permission-enabled, noninteractive `git ls-remote --heads origin main dev`
  succeeded: remote `dev` is `2414d3d`, and remote `main` remains `734a10e`.
- Confirmed a clean local working tree and upstream tracking of `origin/dev`.
- Updated setup and AI context to record that authenticated remote access works.

## 2026-10-06 — Step 1: tracked investigation plan

- Established eight tasks with owners, dependencies, acceptance criteria,
  deliverables, and validation requirements. Task 001 (planning) is complete.
- Task 002 records that the user is building the baseline sandbox save and will
  provide in-game testing. Added a save handoff and observation template.
- Task 003 is ready for the next assistant investigation. Native boarding and
  shared-extender work are conditional on a demonstrated scripting API gap.
- Recorded the user's requirements for suitable-service boarding, varied
  preferences, and a reusable, readable extender with explicit multi-mod
  ordering and conflict semantics if native extension is necessary.
- Validation: eight task records checked for required fields and acceptance
  checklists; 32 local links resolved across 18 Markdown files;
  `git diff --check` passed. No binary analysis or gameplay changes performed.
- Next technical work: task 003. Baseline observations and runtime validation
  remain pending the user's save; static inspection can proceed independently.

## 2026-10-06 — Baseline save received and preserved

- User supplied `TPF3Interlining_baseline`, reporting S1, S2, and S3 sharing
  three stops before branching in different directions.
- Located `TPF3Interlining_Baseline.sav` in the game's local save directory
  within its Heroic prefix. Copied it to ignored
  `analysis/baselines/c228e860e25a/` with a local provenance manifest.
- Verified source/copy SHA-256 equality:
  `c228e860e25a4cc11804e1b3c750793077b5022ea62b41bf568fc7b2f946389c`
  (76,126,628 bytes). Source size and modification time remained unchanged
  during the copy; the original save was not edited.
- Updated the task board, handoff, and context. Task 002 remains in progress:
  exact stop/platform details, vehicle settings, and boarding observations are
  still unverified. No game launch or engine-behavior change was performed.

## 2026-10-06–07 — Scripting bridge and passive runtime probe

- Resumed with the previous session's uncommitted probe/export scripts intact.
  Recorded a durable [assessment](scripting-hook-investigation.md) and
  [handoff](../AI/Context/session-handoff.md).
- Imported the fingerprinted executable into Ghidra 12.1.3. Whole-program
  analysis hit an entry-point error and 240-second timeout; targeted exports
  succeeded using PE function boundaries. Raw evidence stays ignored.
- Traced `OnModifyPathfinding` through its native bridge and return decoder:
  only `town2lineCostFactor` is exchanged. This is insufficient for committing
  alternative boarding. Subscriber ordering, cadence, and full error semantics
  remain unverified; no blanket claim that every scripting approach is exhausted.
- Added reusable Ghidra export scripts and reproduction commands. A follow-up
  targeted run exported `0x140b35820` and `0x140f40f80`; it did not establish
  signal timing or a safe native intervention point.
- Prepared a passive Lua probe for pathfinding, vehicle arrivals, and line-use
  notifications. It leaves input payloads untouched, returns nil, stores only
  its own counters, and samples logs. Shipped script comments distinguish
  line-use start from actual boarding.
- Added a packaging command and disposable-save procedure. Generated ignored
  `analysis/packages/tpf3_interlining_probe_1.zip` with five runtime files.
  User agreed to run the probe; no installation, activation, or game test has
  been performed by the assistant.
- Validation: `rtk proxy lua probes/pathfinding/test_probe.lua` passed; all Lua
  files passed `luac -p`; JSON/resource checks and ZIP CRC validation passed.
  Manifest conventions and resource references were compared with bundled mods.
- Task 003 remains active pending runtime evidence; task 004 is ready because
  the investigated hook lacks a boarding override. Task 005 remains conditional.
  No game executable, shipped resource, or original save was edited.

## Ongoing workflow

Commit coherent research, tooling, and implementation milestones separately.
For each milestone record evidence, commands, outcomes, unresolved assumptions,
and the next validation step. Label static inference separately from runtime
verification. Do not claim installation or gameplay validation without running it.
