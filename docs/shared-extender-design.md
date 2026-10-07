# Shared native extension — draft

The user requested a reusable, readable extension if native access is necessary.
The scripting API cannot currently commit the required passenger change. The
[selected boundary](native-adapter-design.md) provides an initial backend target;
this document specifies ownership and compatibility before writing the loader.
No extension or public API is implemented yet.

## Ownership and clients

One extension owns each engine detour, build verification, engine-thread dispatch,
and modification scope. Mods register named clients and submit typed requests.
Interlining is a client of that service, not the owner of the hook. Client code
does not receive raw engine/component pointers, RTTI, or native function addresses.

Expose a versioned C ABI with explicit structure sizes and fixed-width scalar
fields. Keep any Lua/Teal bridge over the same service layer. Use readable names
such as `register_client`, `subscribe_before_terminal_selection`,
`read_waiting_leg`, and `request_waiting_leg_change`; these names are proposals.
Callbacks receive a temporary context token valid only for their invocation.
Copy returned snapshots; do not retain references into engine storage.

## Dispatch and conflicts

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

The first capability is a bounded waiting-leg replacement with explicit expected
state. Add further capabilities through versioned interfaces, without exposing
generic memory writes. Loader choice, Windows x64 compiler/runtime dependencies,
hook relocation, exception/unwind handling, and unload behavior require a separate
implementation review. GDB remains an observation tool, not the distribution
mechanism for the extension.
