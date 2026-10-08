# 005 — Provide a reusable native extension if required

- Status: In progress — ABI 6 staged changes implemented; first complete alternate-line journey observed
- Owner: Assistant
- Dependencies: 004 verifies a suitable intervention point

## Objective and output

Expose the required engine capability through a readable, reusable interface
that cooperating mods can share. Keep interlining policy in a separate consumer.
Scope the first implementation to verified capabilities; do not promise a
universal solution to unrelated binary patches.

## Acceptance criteria

- [x] Separate extender infrastructure from passenger decision policy.
- [x] Define named APIs/events, documented data types, lifecycle, and examples.
- [x] Version the API and report supported game builds and available capabilities.
- [ ] Support multiple subscribers with deterministic ordering and explicit
      rules for incompatible mutation requests, cancellation, and failures.
- [ ] Specify thread/lifetime rules and validate script requests before mutation.
- [x] Centralize hook installation and detect duplicate ownership where practical.
- [ ] Refuse unsupported native hooks cleanly and retain original behavior when
      the extension cannot safely apply a change.
- [ ] Document save/load behavior, removal/rollback, and compatibility limits.
- [ ] Demonstrate two example consumers coexisting, including an error or
      conflicting request, before claiming multi-mod support.

## Validation and open questions

The [native implementation](../../native/README.md) now provides ABI 1 registration,
unregistration, and copied read-only observations. Two fixture clients coexist in
deterministic order; duplicate registration, expired contexts, wrong-thread calls,
nested dispatch and limits are tested. ABI 6 adds staged mutation requests and
conflict handling below; no Lua binding exists, so broader criteria remain open. Native-client faults/hangs
are outside the current contract; third-party compatibility is unproven.

The production DLL rejects the unsupported fixture executable after remote load.
Windows x64 wrapper tests cover integer/float/stack arguments and original return
value/call count. The loader successfully activated the verified virtual slot in
the disposable game; see the current progress entry for capture results.

ABI 2 now exposes copied waiting-leg, destination and configured line-stop/loading
snapshots, plus bounded scans. Both native suites pass. The in-game capture logged
20 passenger records and 40 stop records without query errors; see
[validation](../../docs/native-query-validation-2026-10-07.md). Observer disabled
automatically; a later optional stop-loader stall is documented and cleaned up.

ABI 3 adds loading-index vehicle lists and copied capacity/onboard/reservation/
selected-terminal fields. The live capture recorded five vehicles without errors;
all sampled reservations were zero. Nonzero/mixed/stale reservations passed
fixtures. See [vehicle validation](../../docs/native-vehicle-validation-2026-10-07.md).

ABI 4 exposes copied direct passenger-edge membership, using StationSystem from
the current callback world. Both suites passed, including two-world separation.
The live capture completed 1,200 callbacks with nine platform records and a
passenger assignment corroborating a nonzero vehicle reservation; see
[platform evidence](../../docs/native-platform-validation-2026-10-07.md).
Endpoint-node fallback remains unsupported; empty direct membership must reject
an initial candidate.

ABI 5 adds copied terminal passenger loading/unloading flags and entity presence
with row/mask consistency. All three native suites pass. A separate consumer
combines the SDK queries into a restricted read-only candidate validator; see
[candidate evidence](../../docs/native-candidate-validation-2026-10-07.md).
The first live S1→S3 candidate passed all implemented checks. Destination presence
alone does not establish onward path validity. ABI 6 subsequently verified one
complete replacement journey, as recorded below. No Lua binding exists.

Next validate repeated replacements and save/reload behavior. The [adapter design](../../docs/native-adapter-design.md) governs scope,
indexing, replication and failure limits.

Implementation history: `git log -- native AI/Tasks/005-shared-extender.md`.

ABI 6 implementation and offline guard/operation tests are complete; the first
[live replacement trial](../../docs/native-mutation-validation-2026-10-08.md)
completed a full alternate-line journey. Broader acceptance criteria remain open.

At sequence 2473 in revision 0.6.1, S1 person 72845 changed to S3 vehicle 97914 and
was observed onboard in both engine worlds. One attempt; expected arrival bytes,
destination 72405 and waiting place preserved. Alighting at planned stop 2 and
arrival at destination 72405 were subsequently observed in both worlds, with no
second engine attempt. Revision 0.6.2's rolling discovery correction is built and
regression-tested separately but not loaded. See the linked trial record for exact hashes and limitations.
