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
  1→2: shared boarding station 97948 terminal 1, matching alighting station 97294
  terminal 0, destination 86002 present and 40 places remaining. This is a proposal,
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

### Trial selection adjustment

The initial live run found valid S3→S1 candidates, excluded solely by the pilot's
S1→S3 selection filter. No engine change had been attempted when preparing the
revision. Built version 0.6.1 separately under `build-v6b` to allow either S1/S3
direction on the same adjacent shared leg. Eligibility, mutation operations and
one-attempt cap are unchanged. User was asked to restart a disposable baseline
for this revised trial; S2's different platform is retained as a negative control.

### First successful live switch

Revision 0.6.1, Windows PID 312, sequence 2473: S1 passenger 72845 qualified for S3
vehicle 97914 on Wimborne→Melksham stops 1→2. One staged request was revalidated
and applied through notified engine operations. Arrival-time bytes, destination
72405 and waiting place were preserved; the scope closed. Subsequent callbacks
observed the passenger **onboard S3 vehicle 97914 in both engine worlds**. Only
one engine attempt was logged. The capture subsequently observed alighting at
planned S3 stop 2 and onward movement in both worlds. A read-only external
follow-up then observed the person at original destination building 72405 in both
worlds. Capture auto-disabled at
3,600 callbacks with no query errors/rejected boundaries; no further changes
were enabled. This establishes the first complete
alternate-line journey; save/reload and broader stability remain untested.

During the run, fixed bounded discovery retaining its first 32 IDs forever.
Revision 0.6.2 (`build-v6c`) admits newer people through rolling replacement and
passes a 96-ID regression plus candidate tests. This consumer-only revision is
prepared, not runtime-tested; the live switch used unchanged revision 0.6.1.

### Save/reload observer prepared

Prepared revision 0.6.3 with an optional observer DLL for explicit passenger and
original-destination IDs. It reuses the verified terminal boundary and copied
queries, discovers world addresses after restart, and dispatches with no writer.
The observer links neither the mutation backend nor candidate policy and omits
the mutation capability. The existing 0.6.2 pilot artifact remains unchanged for
the repeat trial. Added a [checkpoint procedure](save-reload-trial.md) distinguishing
an onboard save from a weaker post-arrival reload check.

Built under ignored `analysis/native-bridge/build-v6d/` with warnings as errors.
Observer fixtures cover distinct worlds, wrong destination, mixed states,
missing/invalid passengers, bounded errors and completion. A host regression
checks that read-only dispatch refuses a change request before validation or
engine operations. All four suites passed, including observer capability and
unsupported-build refusal checks. Invalid build arguments were rejected. Runtime
repeat and save/reload are pending; no new bridge has been loaded. The initial observer build's IDs are previous-trial test inputs.

### Repeat pilot loaded

After the user confirmed the disposable baseline loaded and paused, loaded the
preserved 0.6.2 DLL into Windows PID 312. `tfx_start` returned zero; log
`analysis/native-bridge/build-v6c/tpf-bridge-312-5551718.log` confirms verified slot
installation. Requested normal-speed unpause with readiness to pause/save after
boarding. Replacement and save/reload outcomes remain pending.

### Sparse-demand capture completed; longer window prepared

The user clarified the original baseline has recently established routes and
asked to allow demand more time. The 0.6.2 capture auto-disabled after 3,600
callbacks, 28 validations (one return-leg candidate excluded by the pilot's
selection, seven unsupported legs, twenty boarding mismatches), with zero query
errors/rejected boundaries and no staged request or engine attempt. Only 26
person IDs were remembered, so rolling eviction was not exercised.

Prepared 0.6.4 with an explicit bounded `--observation-limit` option, default
3,600, maximum 36,000; `build-v6e` uses 18,000. Added progress logs every 300
callbacks. Eligibility and the one-attempt cap remain unchanged. Build, candidate
and observer tests passed; invalid limit arguments were rejected. No live memory
or loaded DLL was patched. Asked the user to preserve progressed demand in a new
save before restarting for the longer window.

A subsequent user-reported Wimborne example was inspected with the external
read-only `boarding_snapshot` helper. Person 75515 waited on S3 stops 1→4 beyond
the shared leg; S1 vehicle 98594 shared the boarding platform but did not serve
the planned alighting station. User acknowledged the distinction. One world was
readable; the second was skipped, and the helper reported partial evidence.
[Details, artifact hashes and remaining steps](save-reload-trial.md).

### Longer warm-up trial started

User confirmed save/restart into the progressed baseline. Preserved and verified
`TPF3Interlining_warmup_01.sav` under `analysis/baselines/04569232cb31/` (77,688,477
bytes; full hash in the trial record). Loaded the recorded 0.6.4 DLL into fresh
Linux PID 42420 / Windows PID 312 with `tfx_start=0`. Log
`analysis/native-bridge/build-v6e/tpf-bridge-312-6954251.log` confirms the 18,000
callback cap and observations in both worlds after user-confirmed unpause.
Monitoring for a switched passenger and onboard save checkpoint; outcomes pending.

### Second alternate boarding and saved checkpoint

The longer trial switched S1 passenger 106847 to S3 vehicle 97913 at callback
1,119, preserving destination 85882 and original arrival/waiting place. Both
worlds reported subsequent boarding. One engine attempt; no query errors reported.
User paused and saved `TPF3Interlining_after_switch_01`; preserved and verified its
78,144,391-byte copy under `analysis/baselines/f1b92fb2ec15/`. Read-only snapshots
around the save handoff both confirm the passenger remains onboard in both worlds.

Built and tested a read-only observer specifically for person 106847/destination
85882 under `analysis/native-bridge/reload-106847/`. It advertises no mutation
capability and rejects the unsupported test executable. Requested restart into
the saved checkpoint, paused, to test continuation without the mutation pilot.
[Full evidence, hashes and remaining steps](save-reload-trial.md).

### Onboard save/restart, native transfer and destination arrival verified

Loaded only the passenger-specific read-only observer after user-confirmed
restart into the onboard checkpoint. Fresh Linux PID 44947 / Windows PID 304;
no old bridge mapped. Initial callbacks restored person 106847 onboard S3 vehicle
97913, stops 1→2, destination 85882, in both worlds. The capture recorded alighting,
onward movement and waiting for S2 stops 2→3, then auto-disabled at 3,600 callbacks
with zero query errors/rejected boundaries and zero passenger changes.

Bounded external read-only snapshots subsequently observed S2 boarding (vehicle
98390), onward movement, and original-destination building 85882 in both worlds.
Follow-up 05 exited successfully at 11:19:57 UTC; no watcher remains active.
This completes a second alternate journey and one onboard save/restart with a
native transfer. Partial platform/service snapshots are labeled separately.

Clarified policy scope after the user's question: the adjacent-stop limit applies
to the existing selected train leg, not final destination distance. The original
leg already ended at Melksham; no travel-ahead/wait-for-original-train policy was
added. The complete original onward itinerary and journey-time improvement remain
unverified. Updated policy docs and tasks; next is a bounded multi-passenger trial.

After the reported unexpected power cut, verified the final arrival log, successful
exit metadata, both backup save hashes/sizes, Git object integrity and diff hygiene.
The completed test survived; no replay was needed. Final documentation and handoff
were recovered from working files. [Evidence and hashes](save-reload-trial.md).

## 2026-10-08 — Prepare a five-passenger trial

Preserved and verified the user's expanded signalling/bus feeder save, with
40+ waiting passengers reported. Implemented version 0.7.0 with an explicit
1–5 passenger build cap (default one), a cross-world/client attempted-ID ledger,
one engine attempt per dispatch and fail-closed postcondition handling. The
consumer waits for confirmed expected boarding in both worlds before the next
switch, independently tracks original destinations, and reports incomplete runs.
S1↔S3 adjacent-leg eligibility is unchanged. No bus/platform/through-leg expansion.

All existing native suites, new host/consumer multi-passenger regressions and
observer binary contract tests passed. Added final-callback no-new-proposal test;
invalid passenger cap inputs reject. Expanded baseline and build hashes, commands,
limits and preflight are in [the trial record](multi-passenger-trial.md).
Implementation committed/pushed as `cb7c91d`. Loaded successfully into the
fresh paused game (`tfx_start=0`); startup log confirms configured bounds. The
user has been asked to unpause; live switching/boarding/arrival are pending.

## 2026-10-08 — Four-passenger live result and lifecycle stop

Version 0.7.0 changed four distinct passengers: two S3→S1 on vehicle97745,
then two S1→S3 on vehicles97912/97914. Every replacement preserved expected
age/destination/place and boarded the proposed train/leg in both worlds before
another switch. No repeated passenger or fifth attempt. After callback2118,
eight journey-not-found responses triggered automatic disable; no original
arrival had been recorded. This proves multiple boardings, not batch completion.

Read-only follow-up identified person120770 with an extant entity but absent
simulation-person/journey components in both mask and row handles. Person122261
continued via line79460 then appeared at a building with a different target ID
(122255 versus original72934). Cause and building-identity semantics remain
unresolved. Other passengers continued naturally on S1 and S2. All readers ended.
The user reports no vehicle/line edits, paused and saved; preserved verified
pre-trial and diagnostic backups despite the game's reused save filename.

Version 0.7.1 adds actionable per-person journey-error logs and read-only component
diagnostics without loosening the stop or policy. Separate build succeeded and
consumer regression passed; the new DLL was not loaded. Helper diagnostics ran
read-only against the live missing record. Next investigate native lifecycle
and destination/building identity before another mutation trial. [Evidence,
checkpoint hashes and limitations](multi-passenger-trial.md).

## 2026-10-08 — Inspect passenger identity and current home

Read-only reload/component snapshots found ID120770 now represents a scenery
asset, while person122261 is at its current home122255 with destinations
[122255,75501,-1]. Original target72934 is still a building. Neither finding proves
original-trip arrival or the cause of retirement/retargeting. Targeted Ghidra
exports identify native entity-removal and target-to-home transformation paths.
Added bounded external entity inventory with five passing layout/identity tests.
Restored a separately named pre-trial copy for the next comparison; no mutation
trial resumed. [Lifecycle investigation](passenger-lifecycle-investigation.md).

## 2026-10-08 — Native lifecycle changes reproduced in the control

The preserved pre-trial baseline ran with only a read-only observer, no switching
consumer or mutation backend. Both anomalies reproduced:120770 lost passenger
components while home87062 gained Scaffold;122261 lost work72934 and appeared at
its unchanged home122255 while72934 gained Scaffold. This supports redevelopment
as the explanation without establishing an exact native call sequence. Neither
outcome is original-destination arrival, and full batch completion remains open.

The bounded monitor finished early after560.06 wall seconds (including initial
pause),2,153 accepted world samples and7 rejected inconsistent reads; exit0.
Immutable logs/hashes and limits are in the [lifecycle investigation](passenger-lifecycle-investigation.md).
Targeted Ghidra exports also corroborate native return-home handling. No gameplay
code changed in this milestone. Next add explicit lifecycle classifications to
journey diagnostics with fixtures, retaining mutation bounds and the stop until
handling of these outcomes is validated. User can pause/close the control game.

## 2026-10-08 — Implement lifecycle-aware journey diagnostics

Version 0.7.2 adds ABI7 copied passenger presence and destination slots while
preserving API1–6. The pilot captures an original home/destination baseline and
separates arrivals, both-world removal, both-world invalidation and unresolved
errors. The first tracking anomaly blocks further proposals permanently; neither
lifecycle outcome increments arrivals. Existing mutation and eligibility bounds
remain. No engine transition or raw memory write was added.

Built separately under `analysis/native-bridge/build-v7c` (cap5/18,000 callbacks).
Query, consumer, host batch/failure, registry/hook, candidate and binary contract
suites passed in the isolated Wine prefix. Consumer fixture allowance peaks at
40/64 queries. Final diagnostic log changes were rechecked. [Implementation,
artifact hashes and prepared live trial](lifecycle-tracking.md). The build has
not been loaded into the game; user was asked to restart/load the preserved
pre-trial copy paused for the next test. Live classification remains pending.

## 2026-10-08 — Lifecycle build live startup and first completed journey

Loaded version0.7.2 (`935ef65`, build-v7c) only after verifying the corrected
pre-trial save and fresh process. User confirmed normal-speed unpause. Three
passengers switched and boarded in both worlds:120842/122261 S3→S1 and40365 S1→S3.
All left the replacement trains at their planned alighting stop. At callbacks
2444/2445,122261 was at original workplace72934 in both worlds, after its onward
line79460 leg. This run therefore records an original-destination arrival before
any tracked workplace invalidation; earlier runs must not be relabeled successful.

Capture continues with the existing cap5/18,000. No lifecycle classification is
claimed yet. See [live trial details](lifecycle-tracking.md). Log, process metadata
and active watcher are recorded in the handoff; preserve the loaded DLL.

## 2026-10-08 — Five-passenger lifecycle trial completed

Version0.7.2 confirmed five distinct switches and both-world boardings, followed
by four original-destination arrivals (122261,40365,72701,73064). All five left
their substituted trains at planned stop2. Person120842 then continued on S1,
alighted at its onward stop3, and lost SimPerson before a confirmed78804 arrival.
Callbacks7669/7670 classified removal in both worlds without adding an arrival;
the capture stopped with query_errors0 and switching_blocked1. The wrapper now
forwards only. All external monitors exited; user can pause the game.

Independent copied inventories corroborated120842's non-person state; its exact
removal cause remains untraced. A separate snapshot found122261's later cleared
work assignment/current home, while the final summary retained its earlier
original arrival. This validates removal classification and post-arrival lifecycle
handling live; pre-arrival invalidation remains fixture-tested. Since the five
attempts preceded removal, fixtures establish blocking with unused allowance.
The log audit passed; full outcomes, hashes and limitations are in the
[lifecycle trial](lifecycle-tracking.md). No gameplay code changed in this record.

## 2026-10-08 — Continue after confirmed lifecycle outcomes

Version0.7.3 closes a lifecycle-affected passenger after successful both-world
boarding and matching both-world removal/invalidation, then permits further
eligible passengers within the unchanged five-attempt cap. Pending classifications
pause discovery; a separate permanent query-error block cannot be cleared by
confirmation. Closed IDs remain excluded and attempts consumed. Disagreements,
identity changes and lifecycle loss before confirmed boarding remain stops.
Mixed completed batches report all_outcomes_observed, never all arrivals.

Added default-off trial pacing to reserve the last attempt for a confirmed
lifecycle outcome, addressing the prior capture's already-exhausted allowance.
The prepared build-v7d uses cap5/18,000 with reserve enabled; ABI7 and native
query/mutation operations are unchanged. Normal/reserved consumer tests, host
batch/failure, candidate and DLL contract checks pass; peak allowance40/64.
Invalid reserve/cap combinations reject in CLI and CMake. No live loading yet.
[Behavior, hashes, validation and the next test](lifecycle-continuation.md).

## 2026-10-08 — Continuation trial hit the home/identity guard

Four passengers switched and boarded; the fifth remained reserved. The pilot
stopped with passenger_identity_unresolved after observation2100, no query errors
or confirmed original arrivals/lifecycle outcomes. Continuation is unproven.
Paused read-only inspection found120770 no longer a SimPerson, with the other
three retaining original destination assignments. A transient home change is
implied by the guard, but the specific passenger/value was not logged at the stop.

Added precise identity-stop logging with unchanged policy; normal and reserved
consumer fixtures pass. Separate diagnostic build is not loaded. Current game
bridge forwards only; watcher/readers ended. Full evidence and limitations in
[lifecycle continuation](lifecycle-continuation.md). No save overwritten or new
restart requested. Next establish the exact transition before changing policy.

## 2026-10-08 — Capture the exact home transition

Loaded the tested diagnostic-only build after a fresh restart into the verified
pre-trial baseline. Normal-speed run is active with cap5/18000 and the fifth
attempt reserved. Two passengers have boarded so far. A bounded external copied
read monitor corroborates passenger/home changes alongside the native log.
No policy guard has been relaxed; the home value and final outcome remain pending.
Metadata and current process details are in the handoff and continuation guide.

## 2026-10-08 — Bounded cleared-home handling prepared

Implemented0.7.4 after static analysis identified native home=-1 removal paths.
A cleared home pauses proposals for up to32 callbacks; only matching both-world
removal after confirmed boarding permits continuation. Conflicting identity,
arrival/invalidation, restored home and timeout remain stops. ABI, native writes,
route eligibility and attempt limits unchanged. Consumer/reserve/host/contract
suites pass; max40/64 queries. Build-v7e is NOT loaded.

The current0.7.3 diagnostic run picked40365 instead of120770. Read-only monitoring
still observed120770 removed alongside home construction without switching that
passenger, but missed any transient home value. External monitor finished at600sec;
native trial continues. See [evidence and implementation](home-clear-handling.md).

## 2026-10-08 — Exact cleared-home trigger confirmed

Diagnostic capture stopped at7909 on120842: expected home75531 became-1 while
slots[-1,78804,-1] and target78804 retained the original work destination. Four
switches boarded;122261,40365 and123348 reached original destinations. No fifth
switch, query errors or confirmed lifecycle resolution. A read-only follow-up
~51sec later found120842 without SimPerson in both worlds. Removal latency and
cause remain unmeasured; the32-callback allowance is not live-validated.

All watchers/readers ended. The prepared0.7.4 implementation exactly addresses
this pending state and is pushed as4d6e70a, but is not loaded. Next is its bounded
live test. Complete sequences/hashes in [continuation evidence](lifecycle-continuation.md).

## 2026-10-08 — 0.7.4 live validation loaded

Verified the fresh correct baseline and loaded the tested build-v7e DLL; loader
returned0. Startup confirms cap5/18000, reserve1 and grace32. User confirmed
normal-speed unpause; no live result yet. Process/log details in the handoff and
[cleared-home guide](home-clear-handling.md). No policy or bound changes.

## 2026-10-08 — Home loss persists onboard; classify it separately

0.7.4 correctly blocked new switches after120770's home cleared in both worlds,
then hit its32-callback deadline at2013. Three boardings, no original arrivals
observed, no query errors. Copied follow-up over a minute later still found the
passenger onboard with home=-1/target72649, then recorded both-world removal.
The deadline assumption was insufficient; all watchers/readers have ended.

Prepared0.7.5 closes matching both-world home loss as home_invalidated, preserving
ID exclusion and consumed attempt without claiming removal/arrival. Pending
one-world or mixed states retain the32-callback bound; unknown identity, arrival
and query errors retain their guards. No native writes/ABI/routes/caps changed.
Consumer/reserve/host/contract checks pass (max40/64). Build-v7f is NOT loaded;
next prove actual continuation live. [Complete evidence and policy](home-invalidation.md).

## 2026-10-08 — 0.7.5 live validation started

Verified fresh process, correct original baseline hash and prepared DLL hash;
loaded build-v7f successfully. User confirmed normal-speed unpause. Capture
retains cap5/18000, reserve1 and grace32; live outcomes are pending. Details in
[home-invalidation evidence](home-invalidation.md) and the session handoff.

## 2026-10-08 — Live continuation after removal validated

0.7.5 completed five switches and both-world boardings: four original arrivals,
one separately confirmed removal. After120842 removal at7190,86553 switched at
9214, boarded9247 and reached original86548 at13694/13695. This proves actual
continuation with unused allowance, followed by a complete onward journey.
Stopped all_outcomes_observed, zero query errors; log audit passes, watcher ended.
No home-loss intermediate state was captured, so home_invalidated remains
fixture-tested only. No further running time or replay is needed for this result.

Next implement broader shared-route eligibility while preserving planned alighting
and existing mutation/lifecycle bounds. Full sequences, hashes and limitations in
[the completed trial](home-invalidation.md). No code change during this capture;
existing baseline is unchanged. No stable-release or timing-improvement claim.

## 2026-10-08 — Implement broader shared-route eligibility (0.8.0)

Replaced fixed baseline IDs with bounded demand-line discovery and rolling
line/vehicle caches. Removed the S1↔S3 stops1→2 mutation gate. Original legs may
span one to four forward stop intervals; substitutes preserve the original
alighting stop and ordered station sequence, including express skips. Detours,
extra stops, loops and platform changes remain excluded. Exact copied-route
comparison rejects edits between selection and deferred application.

All nine test invocations pass: route candidates, normal/reserved consumers,
shared host/slot, copied queries/mutation, batch/failure and both DLL contracts.
Worst consumer fixture uses 62/64 queries. Native ABI, hook and mutation operation,
five-person maximum and lifecycle guards are unchanged. Build-v8a is prepared,
**not loaded or live-validated**. No game process or saves were touched.
[Scope, examples, hashes and focused live test](route-expansion.md).

## 2026-10-09 — 0.8.0 route trial started

Verified the unchanged original baseline, fresh process and tested DLL hash.
Loaded build-v8a successfully; user confirmed normal-speed unpause. Limits remain
five attempts/18,000 callbacks, four route intervals, reserve OFF. Capture is
running; multi-stop/express coverage and final outcomes remain pending. Process
and log details are recorded in the handoff and ignored live-run.json.

## 2026-10-09 — Discovery sentinel diagnosed; 0.8.1 prepared

0.8.0 completed one adjacent S1→S3 journey (40365→86033), then stopped at eight
waiting-reader invalid-state errors. No multi-stop or express switch was captured.
Paused read-only inspection found untouched passengers with vehicle sentinel-3;
the frozen discovery cursor identifies86317 as the final candidate. Earlier
analysis identifies separate-2/-3 native branches; neither is ordinary unassigned.

0.8.1 preserves those two values for discovery and excludes them from switching.
Host/backend still require exactly-1; unknown-4 and tracked journey sentinel states
remain errors. New tests verify preservation, continued discovery and zero engine
calls for fabricated sentinel requests. All nine suites pass, peak62/64 queries.
Waiting errors now log person/sequence; exclusion logs and aggregate samples are
bounded. No route/mutation/lifecycle limits changed. Build-v8b NOT loaded.
All readers/watchers ended; original baseline unchanged. See
[full evidence and prepared correction](waiting-sentinels.md).

## 2026-10-09 — 0.8.1 corrected route trial loaded

Fresh process, original baseline hash and prepared DLL verified. Loaded build-v8b
successfully; user confirmed normal-speed unpause. Five attempts/18,000 callbacks,
route bound4 and reserve OFF remain. Runtime sentinel exclusion and broader-route
coverage are pending. Fresh log/process metadata is recorded in the handoff and
ignored live-run.json; no policy change during this capture.

## 2026-10-09 — Corrected trial checkpoint and capacity design

0.8.1 reached9600 callbacks without query errors. One adjacent S3→S1 switch,
both-world boarding and original72934 arrival for122261; four attempts remain.
No special waiting exclusions or multi-stop/express boarding observed yet.
Capture continues unchanged to its existing bound; this is not a final result.

Reviewed the host attempt ledger, result ring and consumer tracking scheduler.
Documented a [proposed bounded capacity implementation](passenger-capacity-plan.md):
separate32-attempt ledger, one pending boarding, four concurrent journey samples,
aggregate reporting and explicit query/failure bounds. No runtime change, build,
injection or save modification was made for this design work.

## 2026-10-09 — Corrected route capture complete

0.8.1 reached its18,000-callback cap with three distinct adjacent switches and
both-world boardings.122261 and128483 reached their original destinations;
75561 was last observed onboard native onward S3/97912 stops2→5, with final
arrival unobserved. No query errors or lifecycle anomalies. Two attempts unused.
The wrapper explicitly forwards only, allowing no further changes.

Audit passes preservation/boarding/uniqueness invariants; all watchers ended.
No sentinel exclusions or multi-stop/express replacements occurred, so those
live-coverage gaps remain. Final counters:963 ready samples,92 validations,
65 boarding mismatches,8 alighting mismatches,14 unsupported legs,2 divergence,
3 eligible. Counters describe repeated checks, not unique passengers.

User asked about sparse matches; explained expanded demand sampling and platform
constraints alongside traffic timing. No timing benefit or RNG-only cause is
established. The third passenger's native onward S3 leg reinforces the need for
future whole-trip comparison. [Sequences, hashes and limitations](waiting-sentinels.md).

No source/binary or game/save modification during capture. Existing nine-suite
test evidence remains applicable; ran the completed-log audit and documentation
diff checks. Next implement the capacity plan and discovery metrics before one
integrated live test. No further running time or save reload is needed now.

## 2026-10-09 — Implement sampled capacity (0.9.0)

Separated the consumer's compact attempt ledger from reusable detailed trackers.
Build caps now permit1–32 attempts; larger trials retain at most four concurrent
journey samples plus one pending boarding. Every change still gets fresh full
validation and both-world boarding confirmation. IDs remain excluded after tracker
release; unsampled boarding never counts as destination arrival. A600-callback
confirmation deadline stops unresolved transitions, including pending results.
Lifecycle guards, shared ABI/hook and native mutation operation remain unchanged.

Added bounded aggregate outcome/discovery metrics. Discovery can use callbacks
formerly reserved for tracking when no trackers remain. No platform/route guard
was relaxed, and increased discovery performance has not been measured live.

All13 native invocations pass; actual consumer fixture confirms32 switches with
four retained samples and28 released boarding trackers, slot reuse/ID protection,
deadlines and truthful totals. Host32-cap/failure/history tests pass. Peak62/64
queries. Nine C/CMake configuration combinations pass expected outcomes, with all
six invalid CLI combinations rejected before building. Corrected pagination in
the expanded fixture during development; no live failure or game change occurred.

Prepared build-v9a,32/18000/four samples, reserve OFF, optional observer only for
contract testing. **Not loaded.** Tests ended; no live watcher or reader started.
[Behavior, hashes and integrated live-test scope](passenger-capacity.md).

## 2026-10-09 — Start integrated sampled-capacity trial

User confirmed the restarted paused game. Verified a fresh process, actual loaded
original baseline and unchanged save/DLL/executable hashes; no older bridge was
mapped. Loaded0.9.0 successfully with32 attempts/18000 callbacks/four samples.
Watcher active; normal-speed unpause requested. Live outcomes and sampled-capacity
coverage remain pending. Fresh process/log metadata is preserved in the handoff
and ignored build-v9a/live-run.json. No reinjection, policy change or save write.

## 2026-10-09 — Sample reuse observed; discovery targeting remains inefficient

At0.9.0 checkpoint6300, two adjacent replacements boarded, one original arrival
was confirmed and the second journey continued on a native onward service.
First tracker released after arrival2420/2421; a later S1 stops4→5 to S3 stops5→6
replacement used the freed sample capacity. Cumulative samples2, peak tracker1,
zero query errors. No unsampled boarding or >5-switch live evidence yet.

User reports50+ people waiting despite sparse matches. At4800,1172 of1489
screening attempts involved another station. This supports improving station-
focused discovery; it does not quantify missed opportunities or establish an
RNG-only cause. Preserve platform/route/capacity checks. Current capture continues
unchanged under its original bounds; this checkpoint is not the final result.

## 2026-10-09 — Stop capture and prioritize station-focused discovery

User reports80+ waiting and wants implementation progress instead of spending
usage on repeated inefficient searches. Called the loaded bridge's tfx_stop;
returned0 at12:21:00Z, then ended the owned watcher. No reload or second DLL.
All three switched passengers reached original destinations, with zero logged
query errors. Latest aggregate snapshot9000 shows active0/peak1 trackers;
last candidate log9419. Manual stopping emits no final pilot_summary, so final
callback/counter values are not fabricated. Separate stopped-audit.json passes
change/boarding/arrival invariants. Higher-capacity/unsampled coverage remains open.

Next implement bounded indexing by boarding station and fresh validation of
indexed candidates, retaining global exploration and all platform/route/capacity
checks. Prepare and test a new DLL before another integrated game run.

## Ongoing workflow

Commit coherent research, tooling, and implementation milestones separately.
For each milestone record evidence, commands, outcomes, unresolved assumptions,
and the next validation step. Label static inference separately from runtime
verification. Do not claim installation or gameplay validation without running it.

## 2026-10-09 — Prioritize station queues and repair discovery omissions (0.9.1)

- User requested addressing sparse candidates with80+ waiting before further
  capture. Implemented16 station buckets ×32 person/line hints, station-first
  screening with global exploration, bounded rotation/eviction and stale-hint
  refresh/removal. Existing route/platform/capacity and deferred validation apply.
- A larger fixture exposed repeated omissions from learning through the churning
  global cache. Discovery now learns directly from each scanned page and rotates
  its position each completed sweep. No cache value authorizes native changes.
- Added indexed/fallback selection, stale-station and index-size counters. Existing
  four vehicle/person screens and one full validation per callback remain; one
  additional copied stop query raises the worst fixture allowance to63/64.
- Warnings-as-errors build and all13 native invocations pass. Each of80 target
  positions is selected on the first loading callback after128 warmup callbacks
  in normal, reserved and sampled modes. Migration, missing records/recovery,
  eviction, duplicate/same-line exclusion, legacy lifecycle and32-attempt sampling
  regressions pass. Lifecycle fixtures now use actual selected IDs because
  station-first discovery changes ordering; outcome assertions were retained.
- Prepared ignored build-v9b:32 attempts,4 samples,18000 callbacks. No game load,
  save edit or new watcher. Previous0.9.0 remains stopped; restart required for
  next integration run. Live search improvement, >5 switches and unsampled
  boardings remain pending. A station window does not enumerate an entire queue.
- Evidence: mutable DLL `a21d9fa8548c43c91be01d4210aff43de7295c872ecc233e3d40c804fd32cacb`;
  final capacity-tests.json `dfa2b4064fb4b0a2d2bfe5b0fb5f64312508ad5af1bbb2c18a9a8e63b8034cb6`.
  station-discovery-hashes.json records artifacts/configuration; all executable
  hashes rechecked against the final13-case report. See docs/station-discovery.md.

## 2026-10-09 — Start the station-focused live trial (0.9.1)

- User confirmed the restarted disposable baseline is ready. Verified fresh
  Linux107579/start1780715, Windows312, actual baseline load12:40:07Z and
  ready12:40:54Z, expected save/executable/DLL hashes and no prior bridge mapping.
- Loaded build-v9b from implementation3cac800; tfx_start returned0 at12:41:51Z.
  Startup confirms station indexing,32 attempts,4 samples,18000 callbacks,
  one pending boarding and600-callback confirmation grace. Requested normal-speed
  unpause without line/vehicle edits; no outcome evidence yet.
- Ignored preflight-process.json, loader-result.json and live-run.json preserve
  startup evidence. Log: `tpf-bridge-312-18170090.log`. The DLL remains mapped until exit.

## 2026-10-09 — Station-focused trial: early boarding and arrival checkpoint

- Normal-speed unpause confirmed. Both simulation worlds observed (thread752);
  watcher54222 and the bounded18000-callback capture remain running.
- By600, three passengers had boarded versus one at600 in the prior0.9.0 run.
  Indexed selections12, wrong-station indexed selections0; fallback87,
  total station mismatches83/screens99. This is a descriptive comparison, not
  a controlled performance measurement or a count of unique rejected people.
- By1800:4 stages,4 successful changes,4 both-world boardings and4 active samples.
  Indexed picks37, hint mismatches0, fallback212, station mismatches195/screens249;
 26 full validations. No query errors.115753 then reached original75525 in both
  worlds at2145/2148.93862,110796 and99871 remain under observation.
- Live-run.json preserves process, unpause and checkpoint details. The first
  arrival releases a sample slot. No unsampled boarding or >5 switches yet;
  replacement legs remain adjacent. Capture is unfinished and left recording
  automatically to avoid repeated empty polling; no new game actions needed.

- While recording the checkpoint,93862 arrived at72810 at2304/2305 and110796
  at72712 at2429/2432. Three original journeys now completed;99871 still tracked.
  Metrics2400:54 indexed picks,1 stale-station rejection,282 fallbacks,
  256 station mismatches/336 screens,28 validations,zero query errors.

## 2026-10-09 — Alternative-platform proposal and capture duration

- User asked how long to leave the current run and proposed reusing native
  alternative-platform walking for cross-platform interlining. At12:54Z the
  latest observation was3300 of18000, roughly11 minutes after confirmed unpause:
  estimate45–50 more minutes at the observed normal-speed rate. The callback cap
  disables further bridge changes; it does not pause/close the game.
- Latest periodic report3000:5 changes and both-world boardings,3 original
  arrivals,2 active samples,zero query errors. Capture remains active; no >5 or
  unsampled-boarding coverage at this checkpoint.
- Reviewed native policy, shipped alternativeTerminals declarations and prior
  walking/arrival/direct-boarding findings. Alternative endpoints are currently
  rejected. Native walking is a promising reuse point; alternate-platform path
  semantics, simulation timing and teleportation are not established. Recorded
  the focused follow-up in task006 without altering this run or claiming support.

## 2026-10-09 — Research native alternative-platform boarding

- User requested research during the active capture. Official manual specifies
  vehicle selection at the last signal for trains, but does not describe
  passenger transfer timing. Targeted read-only Ghidra exports, offline PE data
  and assembly establish a short vehicle-entry walk and a direct boarding fallback.
- Entry search uses transformed vehicle-model paths and a strict5-world-unit
  distance threshold. Empty results return false and the assignment caller invokes
  the common onboard handler. Successful approach creates ModelPerson walking
  state; NoteWalkPersonsArrived completes boarding. Assembly corrects Ghidra's
  inferred void return. No new SDK signature or native operation was introduced.
- This does not prove universal alternative-platform teleportation. Earlier
  passenger relocation/rerouting, route-cost accounting and missed-departure
  recovery remain open. Retain platform guards; plan one isolated same-line
  alternative-platform comparison after the current trial. Full findings,
  addresses, artifact manifest hash and reproduction command are in
  [alternative-platform behavior](alternative-platform-behavior.md).
- All exports finished. No debugger attachment, game/save edits, build change or
  second DLL. Existing0.9.1 capture continues: at6600,seven successful switches
  and boardings,six original arrivals,one active sample,peak4,no pending or
  unsampled boardings,no logged errors. Live >5-switch coverage is now achieved.
  Discovery:123 indexed picks,920 fallbacks,1 stale hint rejection,802 station
  mismatches/1043 screens,71 validations. No final summary or completion audit yet.
- Validation: checked decompiled paths against boolean-return/caller/distance
  assembly and decoded constant; verified documentation links and diff formatting.
  Native tests were not rerun for documentation-only changes.

- User then reported apparent teleport boarding on an excessively long platform
  and hypothesized equivalent alternative-platform behavior. Recorded separately
  from static evidence: consistent with the direct-board fallback, but no timed
  trace or observed alternative-platform case establishes the exact branch or
  simulation-time cost. Native policy and the running capture remain unchanged.
- User clarified certainty about Transport Fever 2. Updated the evidence to
  distinguish predecessor-game precedent from unverified TF3 alternative-platform
  behavior; the TF3 static fallback finding is independent of that recollection.

## 2026-10-09 — Prepare native cross-platform trial 0.10.0

- User requested next-version implementation matching native boarding behavior,
  then clarified S2 uses platform2 in Faringdon and platform3 elsewhere. Added
  same-station boarding and different fixed alighting platforms; no save-specific
  line names, station IDs or passenger IDs appear in eligibility rules.
- ABI8 copies primary/alternative stop terminals and resolves station indices.
  Native registration exports establish Stop alternative vector offset0x10 and
  StationTerminal station/terminal offsets0/4. The new query replaces one stop
  read without increasing query count. Prior ABI layouts/capabilities are retained.
- Actual locked boarding platform must be configured and in the same physical
  station. Waiting membership on another terminal of that station is accepted;
  distinct grouped stations remain excluded. Actual loading/unloading flags,
  capacity, planned endpoint station, destination and route checks remain.
  Alighting uses the engine's destination-driven path builder; different fixed
  platforms are an experiment, not proof of onward reachability. Alternative
  future alighting terminals remain excluded.
- Deferred snapshots now compare configured boarding options and actual selection,
  plus existing route/waiting state. No new mutation operation, forced onboard
  state or walking-time delay. Logs distinguish cross-platform boarding, selected
  alternatives and differing alighting platforms. See
  [scope, build and integrated trial](cross-platform-trial.md).
- Warning-clean final build and all13 native invocations pass, including ABI8
  copied-data/context guards and real-DLL capability checks, malformed/oversized
  options, shared storage, both endpoint platform differences, permissions,
  deferred selection/configuration changes, lifecycle and32-attempt regressions.
  Worst consumer fixture remains63/64 queries. Report executable hashes match
  final binaries; docs links/diff formatting checked. No new live result claimed.
- Prepared build-v10a,32 attempts,4 samples,18000 callbacks. Mutable DLL SHA:
  `a3d77630201f7e0bbed257dcb35b36ecb290bc787c10fef304d5fe0d48a93a35`;
  capacity-tests.json SHA:
  `3c851c5a3dae372b06e07b5e53ccfe748c03a2395edbd365aa5e22831079c3bd`.
  Package instructions and manifest accompany the experimental DLL/loader.
  Native source committed to dev; main remains reserved for stable releases.

## 2026-10-09 — Stop 0.9.1 after eight complete journeys

- User requested cancelling the current trial to move on. Fresh process identity
  matched Linux107579/start1780715. Existing tfx_stop returned0 at13:23:08.647357Z;
  watcher54222 ended intentionally. The old DLL remains loaded until game exit.
- All eight distinct changes boarded and reached the preserved original
  destination in both worlds before stopping. Zero errors. Latest periodic
  metrics11400:8 staged/applied/boarded/arrived,active0,peak4,unsampled0;288 indexed
  picks,1948 fallbacks,2 stale hints rejected,1695 station mismatches/2236 screens,
  170 validations. These are periodic metrics, not an invented exact stop count.
- Separate stopped-capture audit passed all eight change/postcondition/boarding/
  original-arrival checks. No automatic summary; completion-only audit not used.
  Log SHA `1c2f6f7d77e2c888396698edbdc623ee2ea6d184187c79e1bdb5b0e71e9c5272`;
  stopped-audit.json SHA `ba2d2cc0d44f22e5ab5a768b05463553e8b87070b59bf78dea7cd4b6ef11b93e`.
  No unsampled boarding or multi-stop/express live coverage. Next0.10.0 integrated
  trial needs a restart and disposable baseline; no new DLL has been loaded.

## 2026-10-09 — Package and start 0.10.0 integrated trial

- Implementation eabf1eb pushed to origin/dev. Created and verified ignored
  build-v10a/TPF3Interlining-0.10.0-trial.zip (167006bytes), containing tested DLL,
  loader,13-case report,instructions and source/checksum manifest. SHA:
  `c960895b4f8532b7f565fc8a7b5ff3e3a3f2d5a8486fcece1e933633bb26fa2f`.
- User confirmed fresh game readiness. Verified Linux132392/start2068868,
  Windows312, no previous bridge, baseline/game/DLL hashes. Actual stdout load
  before_multi_passenger_01 at13:24:26Z,ready13:25:22Z. Native start returned0
  at13:27:19.156854Z; startup confirms0.10.0,ABI1–8,native_same_station policy.
- User confirmed unpause at normal speed. Watcher55819 active; thread592,
  both worlds observed. New log tpf-bridge-312-20897483.log and live-run.json retain
  evidence. Early callbacks/candidate exclusions only; no new live success claimed.
  Capture runs with32 attempts,4 full samples,18000 callbacks. No line/vehicle edits.

## 2026-10-11 — Recover cross-platform evidence and finish 0.10.1

- Recovered interrupted last-to-first fix from2026-10-09. A shuttle returning to
  its first terminus crosses the line-list boundary (fixture S3:6→0,S1:5→0),
  which0.10.0 rejected. Generic modulo traversal now permits up to4 forward
  intervals on either/both routes, retaining native indices, original destination,
  ordered station sequence and existing platform/capacity/deferred checks.
- Reviewed tests for different route lengths, multi-interval and express crossing,
  missing/invalid stops, repeated stations and actual consumer boarding. All13
  invocations had passed; verified report hashes against executable artifacts and
  CMake reports no work required. Worst query allowance remains63/64.
- Prepared build-v10b/package, not loaded. DLL SHA
  `c26352cba241b8c4bcbc2ad238944caa4e7e3e1670057af1ffb5e673f44f94e8`;
  report SHA `140cea8bd1766cae41a87c5e922188ed70146a919316cb5c68c5a006f0bac4ae`.
  [Scope and integrated trial](return-terminus-fix.md). No stable release claim.
- Recovered0.10.0 log ends at4800:12 successful changes/boardings,5 cross-platform
  boardings,4 sampled original arrivals (including cross-platform105864),4
  unresolved samples,4 unsampled boardings,zero logged errors. No summary/stop
  marker; recording end cause unknown. Partial audit retained in build-v10a.
  Log SHA `e1734ecf1643806831f0bad76e58fb8c18db3cfb8bfda141e69c5a95a9f1f1e3`.
- No current game process found. Await fresh disposable baseline readiness;
  old PIDs/world pointers/watcher details are invalid for the next trial.


## 2026-10-11 — 0.10.1 live trial started

Verified the fresh paused pre-trial baseline, game fingerprint and prepared
artifacts. Loaded commit `4cc0d71` successfully (`tfx_start=0`); user confirmed
normal-speed unpause. Both worlds now deliver successful updates. No switching
or arrival result claimed at startup. Capture and process identity are preserved
in ignored `analysis/native-bridge/build-v10b/live-run.json`; next check is
return-to-first-terminus eligibility, actual boarding and sampled destinations.
Package ZIP checksum: `2c1ad23b592264f564aa383322757440883f4ac6b9f4ec1e5b7e12afe31b547b`.

### 0.10.1 early live evidence and reported S2 miss

At callback 1,800: six both-world boardings, including both S1↔S3 last-to-first
return legs toward Faringdon, three cross-platform boardings and one two-interval
leg. No final arrivals yet; two lifecycle invalidations resolved without counting
arrivals. Capture continues. Preserved user-reported missed S2 departure and a
one-world observational snapshot showing six S3 return-leg waiters, 33 free
places on S2, but loading state already outside the accepted window. Discovery
or scheduling may explain the miss; exact cause remains unproven. See
[checkpoint and limitations](return-terminus-fix.md).

### Paused discovery coverage gap confirmed

After another user-reported S2 miss, inspected the existing paused process using
read-only snapshots. Only one of twelve current S3 return-leg waiters appears in
Wimborne hints; the sixteen-vehicle global cache is full across eleven lines.
The S1 train at Wimborne is absent, while S2 98389 is retained and already
travelling toward Faringdon. This establishes coverage gaps, not the precise
cause of the historical S2 miss. Preserve loading guards; improve bounded
discovery next. No further diagnostic replay needed. Current totals: seven
boardings, two original arrivals, two lifecycle outcomes, three active samples.
Return-leg passenger 118180 alighted correctly at Faringdon and continues by bus.
[Details and evidence limitations](return-terminus-fix.md). Game remains paused.


## 2026-10-11 — 0.11.0 discovery implementation

Fixed the page-skipping discovery behavior: consume every returned ID over
successive callbacks. Expanded bounded station/vehicle retention and added a
sixteen-ID ready priority queue, with up to twelve extra vehicle queries only
when no full validation ran. Fresh eligibility, ABI 8 and native mutation guards
remain. Added ready-opportunity logging and page/priority counters.

All fourteen final native invocations pass. New fixture exercises 241 people,
forty bus stations competing with an eighty-person rail queue, 81 vehicles,
sparse pages, sixteen loading-window phases, repeated switches and stale/deleted
priority hints. Peak 44/64 queries; existing worst case remains 63/64. An early
fixture caught non-ready index entries crowding the priority queue; the final
queue admits only vehicles actually observed in the accepted loading state.
Updated an obsolete scans-equal-callbacks assertion for page draining.

Prepared build-v11a with 32 attempts, four detailed samples and 18,000 callbacks;
no game changes or injection during implementation. Existing 0.10.1 remains
paused pending one restart for the integrated trial. Details, hashes and limits:
[discovery coverage](discovery-coverage.md).


### 0.11.0 loaded into the restarted baseline

Fresh process 44582/start 723426 (Windows 312), actual pre-trial save and all
fingerprints verified. No old bridge mapped. Loading at 10:38:57Z returned
`tfx_start=0`; startup confirms the new discovery configuration and 32/4/18000
trial bounds. Awaiting normal-speed unpause; no live boarding result claimed.
Evidence is under ignored `analysis/native-bridge/build-v11a`.

### 0.11.0 partial live result: S2 return boarding confirmed

After user-confirmed unpause, callback 2400 records twelve changes and twelve
both-world boardings. Five detailed samples include one original arrival
(115753→75525 at 2136–2137), one resolved home invalidation and three active
journeys. Seven boardings are unsampled; no final-arrival claim for those.
No query errors or stop marker at this checkpoint; capture remains running.

Confirmed the user's visible S1→S2 switch at Wimborne toward Faringdon:
108249 boarded98388 in both worlds at1277, preserving the 5→0 leg while changing
both platforms. This is one confirmed passenger, not proof that all21 reported
waiters were evaluated. S1's subsequent arrival does not affect selection;
there is no ETA comparison. Bounded evaluation and boarding confirmation can
limit throughput, but the precise cause of the remaining queue is unproven.
Documented early coverage improvement and limitations in
[discovery coverage](discovery-coverage.md). Documentation-only checkpoint;
the loaded binary and previously passing fourteen-test result are unchanged.

## 2026-10-11 — 0.12.0 boarding-window selection prepared

Implemented the user's request to continue development while0.11.0 completes
its sampled journeys. Per-vehicle general/preferred search cursors prevent
opposite-direction trains from consuming one shared station cursor. Successful
source-leg hints rank subsequent candidates, with one-in-four general exploration;
ready vehicles receive three-of-four first picks. Fresh validation, native
mutation, ABI8, query limits and single pending boarding remain unchanged.

All fourteen native checks pass. The same mixed-direction fixture improves from
7–8 to12–13 switches in120callbacks; both compatible source lines served in all
sixteen phases versus none previously. Simultaneous opposite-direction trials
serve at least ten passengers per train in all eight phases. Worst query budget
remains63/64. These synthetic results do not establish live throughput. Report
hashes verified; details in [boarding-window selection](boarding-window-selection.md).

Prepared separate build-v12a; not loaded. Existing0.11.0 capture remains running,
with20 boardings,4 sampled original arrivals,1 home invalidation and4 active
samples atcallback5400. Eleven other boardings are unsampled. No restart or
additional diagnostic save needed during implementation.

### 0.11.0 readiness check — full switch allowance consumed

At11:08Z, verified32 distinct changes and32 both-world boardings,27 cross-platform.
Twelve lifetime samples:7 original arrivals,1 confirmed home invalidation and4
ongoing journeys;20 other boardings are unsampled. No query/identity/journey
errors or stop marker. Preserved exact log snapshot and structured audit under
ignored build-v11a; [details](discovery-coverage.md).

Recommended proceeding to prepared0.12.0 because the current instance cannot
perform further switches. Remaining samples may finish if left running, but
are not required to begin the next development trial and are not counted as
arrivals. No stop or restart performed.0.12.0 package ZIP verified183299bytes,
SHA256 `e090857f7fbd7a4171c40aa21fdf24171e268d552efef4ff0be937cb6b5a23cc`;
source `dadbab9`, implementation `beadb5c`, already pushed todev.

## 2026-10-11 — 0.12.0 integrated trial loaded

Verified the user-restarted process50787/start946352 (Windows308), original
pre-trial save, executable and prepared artifact/package fingerprints. Actual
save load11:14:02Z, ready11:15:38Z. No previous bridge mapped. Loading at
11:16:57Z returned`tfx_start=0`; startup confirms0.12.0, per-vehicle leg hints
and32/4/18000 bounds. Requested normal-speed unpause; no boarding result claimed
at startup. Watcher17862; evidence under ignored`analysis/native-bridge/build-v12a`.

Previous0.11.0 run ended on restart with final periodic8400 totals unchanged:
32 boardings,7 sampled arrivals,1 lifecycle outcome,4 unresolved samples and20
unsampled boardings. No summary/error marker; retain its partial-run designation.

User confirmed normal-speed unpause. Both worlds deliver successful updates.
First three boardings confirmed:72701 and109709 S2→S1 on98398 atcallbacks201/224,
Melksham→Wimborne;72084 S3→S2 on98388 at308, Melksham→Faringdon. All three
sampled, final destinations pending, no query errors. Repeated switches on the
same train/stop are observed; broader throughput comparison remains open.

### User-observed S1 return departure: three S2 boardings, S3 queue remains

At Wimborne toward Faringdon, user reported16 S1/5 S3/3 S2 waiters and the S2
queue clearing during boarding. Confirmed three S2→S1 boardings on98398 at
1240/1245/1249; no S3 switch logged for that stop. Last validation1258 had15 free
places but picked an opposite-direction S2 passenger and rejected correctly.
Remaining S3 individuals were not exhaustively inspected; exact selection
bottleneck remains unproven. Documented same-source preference/exploration
limitation and preserved raw event underbuild-v12a. Running build unchanged.

### Repeated S3 queue miss; 0.12.0 switch allowance consumed

Second S1 return stop on98594 confirms2 S2→S1 boardings at2342/2349, with36/33
free places at validation and no S3 switch. User updated initial6 S3 waiters to12
remaining. S3 individuals were not exhaustively screened. Attempts25/26 show
the global cap was not responsible; cross-source selection coverage remains
the next implementation focus. Event02 evidence preserved, running code unchanged.

At11:27Z:32 both-world boardings, last2576;7 samples,2 original arrivals,1 home
invalidation,4 active samples,25 unsampled boardings. No logged errors. The
capture can now only complete samples; later missed boardings occur after the
switch cap. Exact snapshot and audit saved underbuild-v12a. No stop command issued.

## 2026-10-11 — 0.13.0 continuous implementation and processing checks

Documented both S3 queue misses before implementation. Endpoint hints now rank
across source lines and prefer a different compatible source queue. All proposals
still pass fresh full validation. The mixed-direction fixture serves20/20 eligible
passengers across16 phases, compared with recorded0.12.0's12–13/20.

Added explicit continuous mode: no lifetime passenger/callback cutoff, recycled
outcome records, bounded samples and sparse per-person waiting-episode guards.
Later journeys require fresh native waiting timestamps and prior both-world
boarding. Host tests cover4,096 episodes; consumer stress stages5,185 requests in
66,500 callbacks, including new demand-free service, deletion/recreation and
stale-request recovery. ABI9 adds paged line discovery; older ABI layouts remain.

Added native query-cost benchmark, live callback/query/read/byte telemetry,
bounded detailed logging and a reusable suite runner. Final copied-memory
measurements: sparse page mean3.769us/p95 3.800us/max1060.600us;64 waiting queries
mean209.622us/p95 396.700us/max979.900us. Synthetic data underWine, not measured game FPS. Existing worst
fixture remains within64queries. All17 final invocations passed; executable and DLL fingerprints verified.
DLL SHA256 `4a39e2765bf0b8388945e71f6722e8533df03b50d27d3494cc151fb2b9fc201f`;
report SHA256 `dd1702a5a0c0aa3dbfde41a929bab33e2b5714e3d94718d2261c8e5e345da5ee`.

See [continuous scope, validation and release gates](continuous-operation.md).
Build-v13a is not loaded; main remains unchanged.0.12.0 log now records a third
original arrival (111528,3156/3157), leaving3 active samples. Its32 switches all
boarded; no new changes are possible in that DLL. Preserved implementation snapshot.

### 0.13.0 candidate packaged and pushed

Implementation `37a21aa` pushed toorigin/dev. Verified200881-byte package
`analysis/native-bridge/build-v13a/TPF3Interlining-0.13.0-rc.zip`, SHA256
`c95368158cc60a9255bca4ee98dcffa1dd47f590f26f6a09735221d57a239199`.
Manifest pins source, executable compatibility and every packaged artifact;
archive integrity and contents verified. No game files/saves bundled.

Requested fresh game restart and paused disposable pre-trial baseline for the
continuous trial. Existing PID50787/start946352 still maps0.12.0;0.13.0 has not
been injected or installed. Main is unchanged; live gate remains pending.

### 0.13.0 continuous trial loaded

User restarted and paused the disposable pre-trial baseline. Verified fresh
LinuxPID68878/start1187665, WindowsPID312, no old bridge mappings, executable/save/
DLL/report/package hashes and actual load11:54:25Z, ready11:55:06Z. Loader returned
`tfx_start=0` at11:56:05Z; log confirms0.13.0/ABI1–9,continuous=1 and lifetime
passenger/callback limits0. The startup-log selector initially included isolated
fixture logs; corrected to the verified WindowsPID312 log without reinjection.

User confirmed normal-speed unpause. First observations succeed in both worlds
(thread592; world1 0x312d2df0,world2 0x312d3ab0). No boarding/performance claim at
this startup checkpoint. Log and identity receipts are under ignored build-v13a;
`watch_live.py` only reads logs/process state and cannot stop or mutate the game.
Monitoring repeated source queues, continuation beyond32 and callback costs.

### 0.13.0 live return-stop coverage still incomplete

User reported S2 arriving Wimborne→Faringdon with3 own waiters,5 S3 and17 S1.
Vehicle98388 had37 spare places at validation; only96845 S1→S2 was staged1235
and confirmed1237. Subsequent S1 vehicle98398 at Wimborne stop5 recorded no
switched boardings. It reported32 free places1486,25 at1495,20 at1503. Logged
checks1486/1490 selected outbound legs and rejected route/alighting mismatches.
The Faringdon-bound S1/S3 passengers are within intended shared-leg policy;
individual records were not captured, so do not claim every displayed waiter
was freshly validated. Preservedwimborne-return-01.json/.log underbuild-v13a.

First window elsewhere did serve both S2 and S3 on S1 vehicle98398 at201/218.
The offline improvement does not resolve the live short-window search gap.
Next fix: establish reachable endpoint hints before the first successful change,
with prefetch work separated from mutation callbacks to retain the query budget.
Current0.13.0 continues unchanged to collect continuous/performance evidence.
At1200callbacks:10 boardings,mean149.366us,p95 upper512us,max1150us,peak43queries/
2123reads/52477bytes. These are bridge callback costs, not whole-game frame times.

### 0.13.1 direction prefetch prepared independently

User clarified all reported passengers are waiting for the shared Wimborne→Faringdon
terminus leg; these belong in the intended candidate pool. Implemented reachable
endpoint hints before the first successful switch at a mixed-endpoint station.
A dedicated prefetch callback preserves the validation budget; unambiguous queues
need no extra delay. Stop/line/count changes and periodic refresh update hints;
fresh route validation remains mandatory.0.13.0 game is unchanged.

Paired short-window regression:0.13.0 switches0/16 windows;0.13.1 serves1–3 in
all16, with64 wrong-direction waiters preceding16 compatible people, two source
lines and80 other vehicles. Existing120-callback case remains20/20; opposing
trains improve minimum10→12 each. All three existing pilot/lifecycle executables
pass unchanged, worst64/64queries. Full final suite runs underbuild-v13b.

Latest0.13.0 checkpoint3600:23 boardings,3 sampled arrivals,4 active samples,
16 unsampled boardings;mean167.476us,p95 upper512us,max1150us,peak46queries.
No stop/error marker. Capture has not yet established live>32 switching.

All17 final0.13.1 checks passed; binary/report fingerprints verified.
- `tpf_shared_bridge.dll` SHA256 `d61dd61ef9c6c506bfcfe4e4c42b54b780673c8771852eacf2a4513da36ad7a7`
- `capacity-tests.json` SHA256 `5b7e076ecc2257cc289d5f907637f882cbde63d890b25c09d4cabd430c42aa77`
- `prefetch-comparison.json` SHA256 `ea379237ce9614f1327c36112088b5c5b27cdf9362c93d17fb7613f808089602`

No0.13.1 injection; current0.13.0 reached28 boardings by4098 with no logged stop.

### 0.13.1 package ready; user restarting

Implementation73f3a32 pushed todev. Verified206200-byte release-candidate ZIP
underbuild-v13b, SHA256 `8bd20680130a230083cc426c13a79c0da1044ddb590173aafbf317693794488f`.
User agreed to restart and leave the same disposable baseline paused; actual
readiness and fresh-process verification remain pending.0.13.1 is not loaded.
Preserved0.13.0 log/audit asbefore-v13b-restart.json/.log before any restart.

### 0.13.1 direction-selection trial loaded

Verified fresh PID77940/start1298881, WindowsPID316, baseline/executable/artifact
hashes and absent previous bridge. Actual baseline load12:12:47Z,ready12:14:17Z.
Loader at12:15:08Z returned0;0.13.1 startup confirms reachable_endpoint_hints,
continuous mode and no lifetime cutoffs. User confirmed normal-speed unpause;
both worlds began observations (0x2cbdda10/0x2cbdcd50). No boarding claim at startup.
Watcher7762 reads logs/process only; log `tpf-bridge-316-13152169.log` and receipts underbuild-v13b.
Live coverage/performance/continuation gates remain pending; main unchanged.

### 0.13.1 Wimborne return: both source queues served, incomplete clearance

User reported S1 approaching Wimborne with12 own waiters,initially2 then5 S3,
and3 S2. After departure they reported4 S3 and3 S2 remaining. Confirmed onS1
vehicle98398:102185 S3→S1 staged1231/boarded1243;75645 S2→S1 1246/1248;
112732 S3→S1 1251/1253. All preserve planned Wimborne→Faringdon endpoint.
Thus two S3 plus one S2 switched; queue-count differences are not identity counts.
Other passengers may arrive while boarding; no individual remaining-queue capture.

First change waited12callbacks for boarding confirmation. Consumer blocks new
switches during that wait. After the third boarding,1254 selected an outbound
S2 leg and rejected alighting mismatch. Sparse logging does not enumerate every
later check, so these are measured delays, not an exhaustive cause attribution.
This confirms improved live direction selection but not complete queue coverage.
Evidence `build-v13b/wimborne-s1-return-01.json/.log` preserves this event.

At1200updates:mean150.243us,p95 upper512us,max1094us,peak43queries/1979reads/
43563bytes. No logged error at the event checkpoint. Current0.13.1 stays loaded;
no0.13.2 implementation or restart requested. Next throughput work should examine
bounded concurrent pending boardings, with fresh capacity/reservation accounting,
per-update query budgets, duplicate guards and exact confirmation timeouts tested
before any live increase. Do not simply remove the pending-boarding guard.

## 2026-10-11 — 0.14.0 concurrent boarding implementation

User requested the next implementation after the partial 0.13.1 Wimborne queue
clearance. Continuous mode now supports four pending boardings, one new request
per dispatch, four full journey samples and per-vehicle conservative seat holds.
Every pending mutation result is checked before another request. Selection and
tracking use separate phases; deadlines, duplicate guards, both-world boarding,
fresh capacity and deferred route/state validation remain. Bounded modes keep
one pending boarding. See [design and evidence](pending-boardings.md).

The exact 0.13.1 consumer stages one passenger in the delayed twelve-callback
fixture; 0.14.0 stages four before any boarding confirmation. Regressions exercise
partial replicas, shrinking capacity, pending/failed results, cancellation/retry,
exact timeout of a later pending passenger, wrong boarding/destination and four
samples alongside four pending passengers. Worst fixture remains 64/64 queries.
Initial test failures exposed inconsistent mocked capacity fields and an early
zero-capacity gate hiding the original rejection reason; both were corrected.

All 18 final native suite invocations pass; report executable/DLL hashes verified.
Continuous stress retains 5,185 requests over 66,500 callbacks; host 4,096 episodes.
Native synthetic 64-query mean190.295µs/p95 280.700µs/max379.200µs; not game FPS.
Prepared separate build-v14a; not loaded. No running DLL or game files changed.

Existing 0.13.1 at4,800 callbacks:31 confirmed boardings,6 sampled original
arrivals,4 active samples,21 unsampled boardings; no query error/summary marker.
Bridge mean167.557µs/p95 upper512µs/max1902µs,peak45queries/2302reads/56041bytes.
Snapshot `build-v13b/v14-development-checkpoint.json/.log` preserves this point.
Do not claim live>32 yet. Main remains unchanged; package and fresh live trial next.

- `tpf_shared_bridge.dll`: `a9649f3bfa4f9885d30d21bfa8513af53ee9dab39ae157040de4aeac1bee915f`
- `capacity-tests.json`: `f596db431dc8b2f968814d98f3e7e1b51a468f4290a02a8fea7e430d168ac3a2`
- `pipeline-comparison.json`: `1cb40519ea57caa9844261db39148ec62138cb46281bdd7aaaa0500fbd1350cc`

### 0.14.0 packaged; user restarting

Implementation9906589 pushed toorigin/dev. Verified202559-byte ZIP
`analysis/native-bridge/build-v14a/TPF3Interlining-0.14.0-rc.zip`, SHA256
`1b7d226644a3f92d35a8e004b82091d96e4fdf97cdfe54f20537df2ad17440ea`.
Manifest pins source, compatibility, tests and packaged artifacts. No game assets
or saves included. User agreed to restart and leave the disposable original
baseline paused; actual readiness and fresh-process verification remain pending.
No0.14.0 injection yet. Preserved old log asbuild-v13b/before-v14a-restart.json/.log.

### 0.14.0 loaded and unpaused; conservative monitoring

Verified fresh LinuxPID88445/start1435269, WindowsPID308, original baseline and
absent old bridge. Save ready13:01:35Z; loader13:02:53Z returnedtfx_start0.
Startup confirms0.14.0, four pending boardings, continuous mode and no lifetime
cutoffs. User confirmed normal-speed unpause and requested conservative checking
with2% usage remaining. A read-only ten-minute collector records periodic totals
and performance; its expiry does not stop the bridge. No outcome claim at startup.

Preflight exposed an inherited incorrect executable fingerprint in the package
metadata and continuous-operation document. Corrected to the actual verified hash
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`;
the DLL's built-in pin and tested binaries were already correct and unchanged.
Updated package202628bytes, SHA256
`ec1cbf6988d9fda2d0ba6d0887a12ebab5fa615d33e73aa788a74c52479a3434`.
This supersedes the previous package fingerprint. Evidence underbuild-v14a.

### 0.14.0 user departure observations — coverage still incomplete

User has1% usage left; preserve evidence and defer further investigation.
Local15:04 S3 left7 S1 waiters, some switched (direction unspecified).
15:07 S2→Faringdon:16 S1/6 S3 waiting, user observed one S1→S2 switch;
subsequent S1 served two S3.15:09 S3 towardMelksham: no observed switches.
15:10 S1→Faringdon:2 own/5 S3/4 S2, user observed no switches.
Do not equate UI counts with individually validated candidates, but shared return
queues remain intended candidates; do not invent exclusions or ETA selection.

One preserved snapshot at13:13:25Z:3000callbacks,24 staged/applied/confirmed
boardings,7 samples,2 original arrivals,1 lifecycle outcome,4 active samples,
17 unsampled boardings. Peak pending3 confirms concurrent boarding is exercised;
capacity_holds0,peaktrackers7. No query-error/timeout/summary markers. Latest
performance2400:mean172.352µs,p95 upper512µs,max14293µs,peak41queries/2145reads/
52753bytes. Investigate the14.293ms outlier; do not claim stable performance.
Evidence:build-v14a/reported-opportunities-01.json/.log. Individual departure
matching and exact causes remain unexamined; no additional mutation or restart.
Next session: correlate these windows with ready vehicles, candidate selections,
tracking/discovery phases and queue coverage before another implementation.
0.14.0 is not stable; preserve current log even if game is closed. User may pause
or quit after this checkpoint. Main unchanged.

## 2026-10-13 — normal mod release and native Linux requirement

User wants a usable mod.io release and faster discovery, with targeted tests.
Clarified that external software/bridge_loader is unacceptable; a normally loaded
shared hook dependency could be acceptable. Current0.14.0 does not meet that
installation requirement. Prioritized establishing native loading over more
Windows-only throughput implementation. No source changes to the native consumer.

Inspected supplied native Linux build; relevant public API declarations match
Windows byte-for-byte. Both executables remove package.loadlib/cpath during Lua
state setup; actual runtime contexts and other loading paths remain unverified.
Prepared and installed a five-file Lua-only capability mod, tested unavailable
globals/one-time logging/no loader invocation. User agreed to launch native Linux
and enable it. No injection, engine mutation or mod.io upload. See
[loading feasibility](mod-loading-feasibility.md) for hashes and evidence.

### Native Linux runtime loading checks completed

Build40420/Schoenbau: package.loadlib and cpath absent in all observed startup
phases. Module searcher table remained, so tested its native entries with an inert
Linux library (only prints, no Lua API calls/hooks/threads/engine access).
After user reload, both native searchers3/4 are nil and handshake unavailable;
the library never loaded. resolve() returns virtual resources, not an OS path.
Evidence underanalysis/runtime-loading. Normal dependency packaging does not
make the current engine hook loadable. No interlining release uploaded.

Corrected the diagnostic once flag after observing resource-module recreation:
private GameScriptState latch prevents repeated update logging; recreation test
passes. Prepared an unsent precise [API request](passenger-hook-request.md).
Current no-external-software release requirement cannot be met by repackaging
0.14.0; supported native loading or passenger mutation scripting remains missing.
