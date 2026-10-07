# 005 — Provide a reusable native extension if required

- Status: In progress — architecture draft; native implementation pending
- Owner: Assistant
- Dependencies: 004 verifies a suitable intervention point

## Objective and output

Expose the required engine capability through a readable, reusable interface
that cooperating mods can share. Keep interlining policy in a separate consumer.
Scope the first implementation to verified capabilities; do not promise a
universal solution to unrelated binary patches.

## Acceptance criteria

- [ ] Separate extender infrastructure from passenger decision policy.
- [ ] Define named APIs/events, documented data types, lifecycle, and examples.
- [ ] Version the API and report supported game builds and available capabilities.
- [ ] Support multiple subscribers with deterministic ordering and explicit
      rules for incompatible mutation requests, cancellation, and failures.
- [ ] Specify thread/lifetime rules and validate script requests before mutation.
- [ ] Centralize hook installation and detect duplicate ownership where practical.
- [ ] Refuse unsupported native hooks cleanly and retain original behavior when
      the extension cannot safely apply a change.
- [ ] Document save/load behavior, removal/rollback, and compatibility limits.
- [ ] Demonstrate two example consumers coexisting, including an error or
      conflicting request, before claiming multi-mod support.

## Validation and open questions

This is an architectural requirement, not an implemented API. See
[agreed requirements](../Context/requirements.md). Exact contracts depend on
the engine findings; compatibility with third-party extenders remains unproven.

The [shared-extension draft](../../docs/shared-extender-design.md) now defines
single hook ownership, a proposed versioned C ABI, scoped callback contexts,
deterministic clients and explicit request conflicts. The
[adapter design](../../docs/native-adapter-design.md) selects a runtime-observed
entry boundary. Next implement pinned bindings and loader/interception without
mutation before enabling the one-passenger experiment. Draft names are not
available APIs, and no acceptance criterion is complete from documentation alone.

Related implementation commits: none yet. Planning history: `git log -- AI/Tasks/005-shared-extender.md`.
