# Native adapter: first intervention

Status: the [native bridge](../native/README.md) implements this entry boundary
and ABI 6 staged engine-owned waiting replacement. Offline guard/operation tests
pass; the [first live trial](native-mutation-validation-2026-10-08.md) is in progress.

## Selected boundary

Intercept `SimEntityAtTerminalSystem::Update` at preferred address
`0x140b0db00` for executable SHA-256
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
At entry, Windows x64 registers contain the system in RCX, engine/world in RDX,
null system-specific update data in R8, and the fourth argument in XMM3.
Preserve the complete ABI; Ghidra's inferred prototypes are not declarations.

Two runtime captures covered 280 complete calls on the simulation thread.
Every observed entry and return had no active modification window, and the
nested/post flag was enabled. The world alternated between two instances.
The dispatcher at `0x142bb8a50` calls the system's Update virtual method;
the observed return address was `0x142bb8aec`.

The adapter should finish its work **before the original function starts**.
That function gathers queue/vehicle candidates before opening its own commit
scope at `0x140b0f409`. Migrating a queue inside that later scope would invalidate
decisions already gathered. The observed empty-window boundary permits proposing
a separate scope; it does not by itself prove cross-line writes correct.

## One-passenger experimental operation

1. Use the current invocation's world; reject reentrancy, non-null update data,
   an existing modification window, unexpected thread/context, or wrong build.
   Never cache an engine pointer across calls.
2. Read and validate one designated passenger and candidate against that world.
   Require the full expected waiting component, including arrival-time bytes,
   unassigned vehicle `-1`, correct current route occurrences, matching boarding
   and alighting terminals, load/unload permissions, and available capacity
   including reservations. Preserve the person's destination and terminal place.
3. Copy the complete 32-byte waiting component. Change only the line and mapped
   boarding/alighting indices; preserve arrival-time bytes and vehicle `-1`.
4. Open a dedicated engine modification scope. Use the engine's component-removal
   and addition functions with notifications enabled. End the scope so posted
   work and notifications finish before the original Update begins.
5. Call the original Update exactly once. Observe assignment, approach, boarding,
   onward travel, and replica consistency. This experiment is one-shot; repeated
   reselection and preference variation are later policy work.

Relevant engine operations are BeginModification `0x142bb4640`, removal
`0x142bb7320`, terminal addition `0x140b21cd0`, and EndModification
`0x142bb4d90`. The game's own scoped wrapper uses `0x142bbbba0`/`0x142bbc060`.
Terminal RTTI is `0x143cea8f0`. These are build-specific research addresses,
not public API addresses. ABI 6 privately binds these four operations after
live signature checks. Add takes a fifth byte argument (local storage=1), verified
at the native remapping call site; it must not be omitted.

## Index and replication responsibilities

Removal/addition must notify both terminal and person systems. Terminal lifecycle
callbacks maintain line/stop queues, ordering, cargo counts, station membership,
and assigned-vehicle membership. Person-system callbacks additionally maintain
line-to-person membership: removal `0x140b2c450` calls `0x140953820`, which asserts
one removal; addition `0x140b2bcb0` calls `0x140951180`, which asserts insertion.
Their storage is copy-on-write. Do not edit these maps directly.

The engine records and replays component operations into another world. A scope
must therefore use engine operations, not overwrite the visible component.
Neither a world address nor a repeated entity ID identifies a unique logical
update across replication. Scope requests to the current callback and revalidate
expected state immediately before applying them.

## Remaining implementation gates

The native bridge now verifies the executable and live virtual table, replaces
Update's slot, and forwards through an ABI-preserving assembly wrapper. ABI 2–5
provide copied passenger/route, vehicle capacity/reservations, direct platform,
terminal permission and entity-presence queries. A separate read-only validator
combines these for the restricted adjacent same-terminal case. Destination
presence does not prove onward reachability; node fallback, fractional load
limits and alternative terminals remain excluded. ABI 6 implements mutation
bindings, full expected-state comparison, deferred conflict checks and one-attempt
limits. Live journey continuation, replication and persistent save behavior still
need evidence; postcondition failures stop further changes but cannot roll back.
Same-line stop remapping demonstrates replacement;
cross-line replacement still needs a disposable-save write test. Removal/addition
is not an established rollback transaction: reject expected failures before the
first removal, and do not promise recovery from an engine assertion or allocator
failure. Verify disabling, save/reload, transfers, and replica behavior before
expanding beyond the one-passenger experiment.

The extension should provide reusable scheduling and scoped services; interlining
policy remains separate. See [the shared-extension draft](shared-extender-design.md).
