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

## 2026-10-07 — First runtime probe results recovered

- Searched the user-supplied Proton prefix and found `local/crash_dump/stdout.txt`.
  UTC timestamps map the user's 13:50–13:56 CEST test to 11:50–11:56Z.
- Preserved the log, probe extract, provenance hashes, and Test1 save under
  ignored `analysis/test-runs/2026-10-07-test1/`. Originals were only read.
- Confirmed build 40408, probe initialization, and all four event types.
  Pathfinding reached logged counter 800; all 28 sampled payloads contained
  an empty town map. Recorded 19 arrivals, 8 line-use-start batches, and 6
  completion batches. Sampling prevents treating 800 as the final total.
- Recorded the user's 13:52 observation: a passenger kept waiting for S2 when
  S1 arrived heading to the same station. The current log does not identify
  the waiting passenger or establish capacity/platform/destination eligibility.
- Verified all five installed probe files match repository revision 1 at
  `game/installation/mods/tpf3_interlining_probe_1/`. Corrected installation
  guidance: that location worked; the earlier user-mod suggestion was untested.
- Added [Test 1](test-run-2026-10-07.md), updated tasks and handoff. Next:
  bounded read-only waiting-state diagnostics and line-name mapping alongside
  native boarding investigation. No behavior change or additional game run.
- Validation: preserved evidence hashes checked, event counts independently
  tallied, local Markdown links and `git diff --check` passed. Documentation-only
  milestone; no repeat of unrelated Lua or Ghidra checks required.

## 2026-10-07 — Arrival diagnostics revision 2 and native boarding trace

- User authorized continued implementation at the assistant's discretion.
  Added read-only arrival snapshots with line/station names, configured/actual
  terminals, passenger capacity and occupancy, load settings, and other lines'
  waiting queues. Each sampled passenger includes planned line and alighting
  station, enabling correlation that revision 1 could not provide.
- Bounded traversal to 16 queues, 8 people per queue, and 32 people per arrival;
  first 60 arrivals then every 100th. Native queries may return full lists.
  Added error isolation and one-time counter reset on revision upgrade.
- Validation: both Lua suites passed, covering index/cargo mapping, differing
  platforms, stale entities, query errors, sampling limits, revision migration,
  nil returns, and unchanged routing data. All Lua files passed syntax checks;
  package manifest/resource and ZIP CRC checks passed.
- Installed revision 2 in the existing custom probe mod after finding no running
  game process and verifying both old files against repository revision 1.
  Backups and hashes are in ignored `analysis/probe-install-backups/revision1-to-2/`.
  Installed files were compared with source after replacement. New diagnostics
  need a game run; no executable, shipped resource, or save was changed.
- Located native terminal update, path-change notification, done-waiting, and
  vehicle-arrival functions through diagnostic references and PE ranges.
  Targeted Ghidra exports succeeded; [native trace](native-boarding-investigation.md)
  records addresses and remaining uncertainty. The terminal update queries a
  two-part vehicle key and checks cargo/loading data. The done-waiting path
  reaches `SimPersonDrivingArriveAtEndOfPath`, so it is not established as a
  transit boarding hook.
- Task 004 is now active. Next: verify revision 2 snapshots and trace vehicle
  selection through engine-owned state commits. No native extender or boarding
  behavior change has been implemented. Documentation links and diff checks pass.

## 2026-10-07 — Test 2 validates queue snapshots and captures a reference case

- Collected the user's 14:19–14:26 test from the known Proton log path. Preserved
  log, probe extract, Test2 save, and hashes under ignored
  `analysis/test-runs/2026-10-07-test2/`; original files were only read.
- Revision 2 produced 18 arrival snapshots, 79 queue snapshots, and 19 waiting
  records without snapshot errors or limit messages. Mapped S1=`98278`,
  S2=`97742`, S3=`97743`. Pathfinding again reached counter 800 with empty
  town maps in all logged samples.
- Identified person `75625` queued for S1 Wimborne-to-Melksham when S3 arrived
  at the same terminal at 14:19:53 with capacity 40, occupancy 3, and loading
  enabled. The same passenger remained queued when S1 arrived 14:21:47 and
  started S1 usage at 14:21:52. The earlier S3 vehicle reached Melksham next.
- Preserved the user's near-14:20 S2 observation separately: the nearest logged
  Wimborne arrival is S3, and departure times are not logged. S2 is configured
  at a different Wimborne terminal. A second person, `87485`, is a plausible
  match for the near-14:25 S1 boarding report, with line-use start at 14:26:00.
- Added [Test 2](test-run-2026-10-07-2.md) and updated the handoff/task board.
  First intervention scenario: S1/S3 with matching boarding/alighting terminals;
  retain S2 for later platform-transfer cases. Full engine eligibility remains
  unproven; no boarding fix or new installation was performed this turn.
- Validation: evidence hashes, exact selected event correlations, event counts,
  local Markdown links, and diff checks passed. Next: continue task 004's native
  selection/transition trace; another generic probe run is not needed yet.

## 2026-10-07 — Trace boarding transitions and implement offline candidate policy

- Followed the line/stop vehicle lookup through assignment. Confirmed the
  `SimEntityAtTerminal` vehicle store at `0x140b0f49f` and its pre/post-change
  notifications. Those callbacks update a vehicle index; separate lifecycle
  callbacks maintain line/stop queues and arrival-time ordering.
- Traced assigned-passenger and walking-arrival paths to `0x140b374c0`.
  It carries line/boarding/alighting indices into onboard state and adds
  `SimPersonAtVehicle` and `SimEntityAtVehicle`, confirmed through RTTI.
  Seat selection was located; reservations, onward journeys, safe mutation
  boundaries, and failure recovery remain unresolved. A widened vehicle list
  or isolated line-field write would not coordinate this state.
- Added `tools/native_references.py`, a read-only PE diagnostic-reference
  helper. Candidates were checked in Ghidra/disassembly. Documented that PE
  exception regions can be cold assertion fragments rather than function entries.
  Raw exports remain ignored under `analysis/native-boarding/`.
- Added pure Lua candidate policy under `src/interlining/`, separate from
  engine bindings. It proposes only an adjacent, matching-terminal segment
  with an unassigned passenger and observed capacity; incomplete or unsuitable
  inputs keep the current journey. Every proposal requires native revalidation.
  There is no service ranking, preference variation, reservation, or engine write.
- Validation: `lua tests/test_candidate.lua` passed the reconstructed Test 2
  S1/S3 case, 25 rejection controls, and input immutability checks. Both new Lua
  files passed `luac -p`. Targeted Ghidra exports succeeded. Python syntax,
  documentation links, and diff checks were checked before committing.
- Updated the native trace, task board, and handoff. Installed probe remains
  revision 2; no game binary, installed mod, or save was changed. No additional
  user test is needed until runtime instrumentation or an adapter is ready.
  Next: verify queue migration and the engine-owned callback boundary.

## 2026-10-07 — Verify ordinary boarding with native runtime observation

- Found the engine's same-line stop-remapping branch in `NoteLineChanged`:
  remove/re-add `SimEntityAtTerminal`, preserving arrival-time bytes and vehicle.
  Added evidence for the person-system line index and destination-driven onward
  path construction. Cross-line reassignment remains untested.
- Identified BeginModification, EndModification, Post, and Replicate. Component
  mutation requires an existing modification window; Post additionally requires
  the nested/post flag. Replica replay repeats component operations in another
  engine instance. Do not bypass these checks or cache one world as authoritative.
- Implemented a bounded GDB observer with four hardware breakpoints, executable
  hash/live signature validation, relocation support, copied-byte decoders,
  thread/caller/mutation-flag logging, and automatic breakpoint cleanup.
  It uses no engine calls or passenger-state writes and installs no game files.
- The user loaded and paused a disposable save. Initial capture was empty while
  paused; interrupted and detached cleanly. Changed the observation timer to
  start at the first hit. Reattached after the user unpaused at normal speed.
- Capture 2 logged nine relevant records: person `71583`, S3 segment 3→4,
  vehicle `97914`; waiting, approach, and onboard entries matched static layouts.
  All records came from simulation thread `75558`. Two engine instances and
  replica call sites were observed. Capture stopped after 5,001 breakpoint hits
  in about 11 seconds and detached. See [evidence](native-runtime-observation-2026-10-07.md).
- Validation: four decoder unit tests passed, GDB loaded the command, source/live
  executable checks passed, and native observation succeeded. Targeted Ghidra
  exports completed. Updated research, task board, and handoff. No behavior fix
  is active; the offline policy and installed passive probe are unchanged.
- Next: narrow observation to terminal-update/modification boundaries and finish
  cross-line index-removal semantics before implementing the mutation adapter.

## 2026-10-07 — Select the pre-Update boundary and draft the shared extension

- Traced terminal Update's scoped commit: selection precedes BeginModification,
  and EndModification drains posted work before returning. This rules out queue
  migration inside the already-selected assignment batch. Verified the system
  dispatcher and person-system line-index removal/insertion callbacks.
- Added `boundary` mode to the native observer: Update entry, commit-scope open,
  scope close, and return. Correlates per thread/stack and rejects inconsistent
  phase/world data; contexts expire at return. Existing boarding mode is retained.
- User reopened a disposable save and unpaused after attachment. Two captures
  paired 80 and 200 complete calls on the simulation thread, alternating two
  worlds. All entry/return samples had a null modification window and nested/post
  flag one. No scope-open/close breakpoint fired; those interior sites remain
  unvalidated. Both captures detached; the game reported no attached debugger.
- Selected Update entry as the candidate: validate a single waiting leg, perform
  engine remove/re-add inside a separate modification scope, close it, then call
  original Update once. This is a concrete proposal, not a validated write path.
  [Adapter design](native-adapter-design.md) records bindings, index obligations,
  revalidation, preserved state, and remaining implementation gates.
- Drafted [shared-extension ownership and API rules](shared-extender-design.md):
  centralized hooks, versioned C ABI, temporary contexts, deterministic clients,
  and explicit request conflicts. Task 005 now tracks design work; no extension
  or proposed API is implemented yet.
- Validation: eight Python decoder/correlation tests passed, GDB loaded the new
  command mode, live build/site checks passed, and entry/return captures succeeded.
  Targeted Ghidra exports completed. Raw captures/exports remain ignored.
- Next: implement pinned native bindings and the shared loader/interception with
  mutation disabled, then validate a one-passenger cross-line replacement. No
  installed mod, game binary, or save was modified by the assistant.

## 2026-10-07 — Implement and validate the read-only native bridge

- Implemented a shared C ABI with named clients, deterministic callback order,
  copied observations, per-client expiring contexts, thread/reentrancy guards,
  and bounded registration. Passenger policy remains separate; no mutation API
  or Lua binding is exposed.
- Identified the terminal virtual table and RTTI. Added full executable/live
  checks and atomic expected-pointer slot replacement, per-process ownership,
  original protection restoration, and an assembly forwarding wrapper with
  Windows unwind metadata. No engine instructions are relocated.
- Added an explicit DLL loader, stop control, bounded 400-call observer, and
  documented process-lifetime pinning. No proxy DLL, game-file replacement or
  automatic startup is installed. The original engine call runs after observation
  or rejection. Downloaded/verified a local pinned LLVM-MinGW toolchain and added
  a reproducible build helper; binaries/toolchain remain ignored.
- Validation: warning-clean Windows cross-build; isolated Wine tests passed
  two-client ordering and invalid access, protected slot ownership, arguments,
  original return and exactly-once forwarding. Production DLL refused the
  unsupported fixture after remote load. Build helper/syntax checks passed.
- Loaded into the existing disposable game using its matching Proton runtime.
  The user unpaused; the bridge accepted 400 calls and auto-disabled observation.
  Samples show one simulation thread, alternating worlds and zero rejected
  boundaries/dispatch errors. Explicit stop returned zero. See
  [full evidence](native-bridge-validation-2026-10-07.md) for artifact hashes and
  validation limits. No passenger, queue, save or installed probe was changed.
- Next: expose copied live waiting-leg/route/capacity queries, validate a single
  candidate, then implement the engine-owned replacement experiment. Native
  clients remain trusted code, and arbitrary callback failures are not contained.
  No interlining behavior fix or stable release is claimed.

## 2026-10-07 — Add and validate copied passenger and route queries

- Traced the component registry, entity component handles, contiguous/shared
  storage and native line-stop field registrations. Implemented bounded memory
  readers that avoid the game's asserting getters and return copied values.
- Added ABI 2 while preserving ABI 1: waiting-leg/destination snapshots,
  configured stop/station/terminal/loading snapshots, and paged waiting-person
  scans. Enforced callback/thread lifetime and shared per-dispatch query/read/byte
  budgets; failed queries leave results unchanged. No mutation capability added.
- Added an example consumer using only the public API. It scans up to 1,024 entity
  masks per callback, returns at most 16 IDs per page, and records at most 24
  passenger samples within the existing 400-call capture.
- Validation: warning-clean Windows cross-build; query fixtures and existing
  bridge tests passed under isolated Wine. Tests cover both storage layouts,
  malformed/missing data, loading flags/fractions, pagination, limits, and
  context/thread access. User restarted and resumed a disposable save. Capture
  reached 400 callbacks with 20 passenger records (8 IDs), 40 stop records and
  no sampled query errors/rejected boundaries. Confirmed the known S2/S3
  Wimborne terminal distinction. [Evidence](native-query-validation-2026-10-07.md)
  records artifact hashes, static provenance and validation limits.
- Observer auto-disabled. The optional stop-confirmation loader hung under Proton;
  direct bridge-counter reads confirmed disabled/400/zero rejected. Terminated
  only that loader; root cause remains unresolved. Future loader calls need an
  outer timeout covering Wine startup/API stalls. No game/save/probe file or
  passenger state was modified; DLL remains forwarding until game exit.
- Next: vehicle capacity including reservations and actual platform compatibility,
  then full candidate validation before the single-passenger write experiment.
  Maximum-load fractions and configured terminals alone are not eligibility.

## 2026-10-07 — Add copied vehicle capacity and boarding-reservation queries

- Added ABI 3 with loading-index vehicle lists and copied vehicle snapshots:
  capacity, onboard/reserved counts, remaining places, selected arrival terminal,
  decision lock, raw state/loading values and passenger loading timer. ABI 1/2
  remain compatible. All operations use current-callback copies and existing
  budgets; failed or oversized queries preserve output and never truncate.
- Traced engine occupancy and assignment indexes. Reservations include waiting
  passengers already assigned a vehicle. Arrival station is a station-group
  index, not an entity ID; this distinction is explicit in the SDK.
- Validation: warning-clean cross-build, new reservation/corruption/ABI fixtures
  and existing bridge/query tests passed. User restarted/resumed a disposable
  baseline. Capture completed 400 callbacks with 10 snapshots across five
  S1/S2/S3 vehicles, 15 passenger records, 30 stop records, no sampled query
  errors and zero rejected boundaries. Observer auto-disabled; no stop-loader
  or debugger required. [Evidence](native-vehicle-validation-2026-10-07.md)
  records hashes, native layouts, examples and limitations.
- Every live reservation sample was zero; nonzero reservations are fixture-tested,
  not yet independently confirmed live. Actual passenger platform access remains
  unimplemented. No passenger mutation, installed probe change or save write.
- Corrected the outdated version banner after capture in a separate final build;
  retained the loaded DLL and original log unchanged. Next: finish waiting-edge
  terminal membership and nonzero-reservation validation, then candidate checks.

## 2026-10-07 — Validate direct platform membership and live reservations

- Added ABI 4 copied passenger-edge/platform membership, preserving ABI 1–3.
  Located StationSystem through the current world's system vector and verified
  vtable identity, avoiding the scripting context's possible world mismatch.
  Reader supports multiple memberships, explicit empty/missing/error results,
  bounded maps and unchanged output on failure.
- Traced StationSystem insertion and waiting-place registration. Confirmed that
  platform pairs contain station entities, whereas vehicle arrival uses a station
  index. The engine has an endpoint-node fallback; this API exposes direct edge
  membership only, so empty results cannot authorize boarding.
- Both native suites and warning-clean cross-build passed. New tests distinguish
  two worlds with identical person/edge IDs but different terminal memberships,
  and cover malformed/missing/duplicate/oversized data and ABI 4 guards.
- User restarted/resumed a disposable baseline. The revised bounded consumer
  completed 1,200 callbacks: nine platform snapshots, 15 shared-platform line
  comparisons, 64 vehicle snapshots across ten IDs, four nonzero-reservation
  samples and one assigned-passenger cross-check. No logged query errors or
  rejected boundaries. Auto-disabled without debugger or stop-loader.
- Live example: person 71583 was assigned to vehicle 97913 at sequence 595;
  separate vehicle read reported capacity 40, onboard 0, reserved 1, remaining 39.
  Wimborne S1/S3 waiting edges map to terminal 1; S2 maps to terminal 2.
  [Evidence and hashes](native-platform-validation-2026-10-07.md).
- Next: read-only candidate validation including terminal load/unload flags,
  locked actual arrival and adjacent journey continuity, then the bounded
  engine-owned replacement experiment. No passenger mutation or behavior fix
  is active; installed Lua probe and game/save files remain unchanged.

## 2026-10-07 — Validate a live read-only alternate-boarding candidate

- Added ABI 5 terminal passenger load/unload flags and entity-presence checks,
  preserving ABI 1–4. Verified Station/terminal layouts through typed access and
  field registration. Presence checks component-row/mask consistency without
  claiming destination reachability.
- Implemented a separate native interlining consumer using only the public SDK.
  It re-reads current passenger/route, direct platform access, locked ready
  arrival, terminal permissions, destination presence, loading-index membership
  and capacity after reservations. Positive proposals preserve expected waiting
  state and require engine revalidation; no mutation API was added.
- Restricted the pilot to adjacent forward same-terminal legs, load mode0 and
  100% maximum load. Multi-stop/wrap/alternative-terminal/node-fallback cases
  remain excluded; fractional native compartment allocation is unresolved.
- Validation: warning-clean build; bridge, query and candidate suites all passed
  under isolated Wine. User restarted and unpaused a disposable baseline. The
  bounded capture completed 1,200 callbacks, 27 candidate evaluations, seven
  positives for the same observed pair, 12 unsupported-leg rejections and eight
  boarding mismatches. Zero query errors/rejected boundaries; auto-disabled.
- At sequence 47, S1 person 22747 passed the checks for S3 vehicle 97959, stops
  1→2: shared boarding station 97948 terminal1, matching alighting station97294
  terminal0, destination86002 present and 40 places remaining. This is a proposal,
  not an actual alternate boarding. [Evidence and hashes](native-candidate-validation-2026-10-07.md).
- Updated SDK/policy docs, adapter gates, tasks and handoff. Next implement the
  bounded engine-owned waiting replacement, then verify actual boarding,
  alighting, continuation and replica consistency. No game/save/probe file was
  changed; no passenger preference policy or stable behavior fix is active.

## 2026-10-08 — Implement the bounded engine-owned passenger switch

- Added ABI 6 staged waiting-line requests and owner-scoped outcome tickets,
  preserving older interfaces. All requests wait until observers finish;
  same-person conflicts reject all contenders. A fresh callback revalidates
  eligibility, and the backend compares the complete 32-byte waiting component,
  destination and place before invoking any engine change.
- Bound and verified Begin/Remove/Add/End against the pinned executable and live
  entry bytes. Confirmed the add function's fifth stack argument from native
  same-line remapping. Preserve all original waiting bytes except the three route
  integers; use notified engine operations and finish the scope before selection.
- Enforced one engine attempt per DLL lifetime across worlds and clients. Added
  copied journey-state queries and a baseline S1→S3 pilot consumer that tracks
  boarding, leaving the vehicle and original-destination building presence.
- Validation: warnings-as-errors build and all three isolated Wine suites pass.
  New fixtures cover conflicts, ownership, thread/context expiry, deferred order,
  stale raw padding/destination, policy refusal, native-call argument/order,
  preserved bytes, postconditions and the process-wide attempt allowance.
- Loaded build-v6 into the user's paused disposable save; user unpaused.
  Live trial is running. [Implementation, hashes and trial record](native-mutation-validation-2026-10-08.md).
  No successful gameplay change is claimed until the runtime result is recorded.

## Ongoing workflow

Commit coherent research, tooling, and implementation milestones separately.
For each milestone record evidence, commands, outcomes, unresolved assumptions,
and the next validation step. Label static inference separately from runtime
verification. Do not claim installation or gameplay validation without running it.
