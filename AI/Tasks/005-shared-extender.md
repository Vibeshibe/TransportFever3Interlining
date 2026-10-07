# 005 — Provide a reusable native extension if required

- Status: In progress — read-only native bridge and C ABI implemented
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
nested dispatch and limits are tested. No mutation request/conflict API or Lua
binding exists, so the broader criteria remain open. Native-client faults/hangs
are outside the current contract; third-party compatibility is unproven.

The production DLL rejects the unsupported fixture executable after remote load.
Windows x64 wrapper tests cover integer/float/stack arguments and original return
value/call count. The loader successfully activated the verified virtual slot in
the disposable game; see the current progress entry for capture results.

Next implement copied live waiting-leg/route queries, then validate one bounded
cross-line replacement. The [adapter design](../../docs/native-adapter-design.md)
still governs mutation, indexing and replication requirements.

Implementation history: `git log -- native AI/Tasks/005-shared-extender.md`.
