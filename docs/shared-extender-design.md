# Shared native extension — implementation and remaining design

The user requested a reusable, readable extension if native access is necessary.
The scripting API cannot currently commit the required passenger change. The
[selected boundary](native-adapter-design.md) provides an initial backend target;
this document specifies ownership and compatibility. The first
[native bridge](../native/README.md) implements a versioned C ABI, explicit loader,
and verified virtual-table interception. ABI 6 stages one experimental passenger
replacement; Lua bindings remain unimplemented.
ABI 2 adds [copied passenger and line-stop queries](../native/README.md), with
bounded scans and per-dispatch read budgets; ABI 1 remains compatible.

## Ownership and clients

One extension owns each engine detour, build verification, engine-thread dispatch,
and modification scope. Mods register named clients and submit typed requests.
Interlining is a client of that service, not the owner of the hook. Client code
does not receive raw engine/component pointers, RTTI, or native function addresses.

ABI 1 uses explicit structure sizes and fixed-width scalar fields. Its concrete
operations are `register_client`, `unregister_client`, and `read_observation`.
Keep any future Lua/Teal bridge over the same service layer. Use readable names
such as `register_client`, `subscribe_before_terminal_selection`,
`request_waiting_leg_change`; mutation operations remain proposals. `read_waiting_leg`,
`read_line_stop` and `scan_waiting_people` are now implemented in ABI 2.
Callbacks receive a temporary context token valid only for their invocation.
Copy returned snapshots; do not retain references into engine storage.

## Dispatch and conflicts

The host implements ordering, context/thread rules, staged requests and explicit
conflict outcomes. ABI 6 rejects all same-person contenders before applying any
change and calls the requesting consumer again for eligibility revalidation.
Version 0.7 uses an explicit build-time cap of 1–5 distinct person attempts per
DLL lifetime (default one), and at most one engine attempt per dispatch. An
attempted-ID ledger prevents repeating a passenger across worlds or clients.
Failed operation postconditions latch further writes shut. These host guards are
separate from the interlining consumer's requirement to observe boarding in both
worlds before selecting another passenger. [Trial details](multi-passenger-trial.md).

Publish callback order explicitly and keep it deterministic by client identifier.
Reject duplicate identifiers and registration changes during dispatch. Stage
requests until callbacks finish, then validate/apply them through the host.
For the first implementation, reject competing requests for the same passenger
within one dispatch; do not silently let the last mod win. Clients get explicit
outcomes such as stale state, conflicting request, unsupported build, and applied.

Only the native backend may open a modification scope. A callback may not nest
another dispatch, mutate while a snapshot is being collected, or invoke engine
functions from another thread. Context expiry and per-dispatch limits must be
enforced, not left to client convention.

## Compatibility and release scope

Pin engine layouts and signatures to an executable fingerprint. Unsupported
builds disable capabilities with a clear diagnostic. Detect an already-modified
hook site and decline installation rather than overwrite another extension.
One shared registry reduces conflicts between cooperating clients; it cannot
guarantee compatibility with independent binary patches.

The first capability was read-only terminal-entry observation. ABI 6 now adds
bounded waiting-leg replacement with explicit expected state. Add capabilities through versioned interfaces without exposing
generic memory writes. The pilot uses LLVM-MinGW, an explicit Windows loader,
atomic virtual-slot replacement, and an assembly wrapper with unwind metadata.
No instructions are relocated. The DLL stays pinned until exit; disabling stops
observations but leaves forwarding in place. Native callbacks must return
normally; fault/hang containment is not provided. See the implementation README
for lifecycle and failure rules. GDB remains a separate observation tool.
