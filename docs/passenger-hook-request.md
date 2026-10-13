# Proposed scripting capability for interlining

Draft for discussion with Urban Games; not sent or published.

## Requested capability

Expose a supported command that changes a waiting passenger's current public
transport leg to a compatible arriving vehicle/line while retaining the final
destination and onward journey. Existing Lua APIs expose station/line-stop queues
and arriving vehicles but no corresponding waiting-leg replacement command.
Normal mods cannot load our native implementation: Linux build40420 removes
`package.loadlib` and the native module searchers.

## Concrete reproduction

S1, S2 and S3 share three stops before branching. At Wimborne, passengers planning
to alight at the shared Faringdon terminus remain bound to S3 even when a compatible
S1 or S2 train is boarding with spare capacity. A native prototype has demonstrated
alternate-line boarding and sampled original-destination arrivals, but relies on
Windows-specific engine access and an external development loader.

## Minimum command contract

For example, a versioned `tryReplaceWaitingLeg` could accept:

- Passenger ID and expected current line, boarding/alighting indices and waiting
  timestamp, so a stale script proposal is rejected.
- Candidate vehicle, substitute line and boarding/alighting indices.
- A completion result distinguishing applied, stale, incompatible and no-capacity.

The engine should revalidate the passenger, vehicle, direction, allowed boarding
and alighting terminals, seat availability and original journey endpoint at commit.
It should own queue/index updates, replication and save serialization. Preserve
waiting age and final destination. Returning success should mean the change was
committed; later boarding/arrival observations remain separate.

## Script scheduling

An arriving/boarding vehicle notification plus existing direct queue queries lets
mods inspect the affected station instead of slowly scanning all world entities.
A bounded batch command, or several requests processed under an engine budget,
would support busy stations. Reject duplicates and resolve competing requests
predictably. A documented maximum batch size and callback thread/context are
preferable to independent native hooks from each mod.

A narrow engine-owned Lua command would support Windows and native Linux without
external loaders or arbitrary native-code access. An official native-plugin API
could be an alternative, but is a broader compatibility commitment.

## Evidence

See [normal-mod loading investigation](mod-loading-feasibility.md) and
[0.14.0 boarding implementation](pending-boardings.md). This request proposes an
API; it does not claim the command already exists or that all alternative Lua
approaches have been exhausted.
