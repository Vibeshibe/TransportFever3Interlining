# Scripting-hook investigation

## Assessment

Static inspection of this build shows that `OnModifyPathfinding` exposes a
`town2lineCostFactor` map. It cannot by itself select an arriving vehicle for a
waiting passenger or commit an alternative journey. No supported passenger
reassignment command was found in the inspected public declarations. This is a
specific API gap, not proof that every possible scripting approach is exhausted.

The next checks are runtime observation with the passive probe and investigation
of native boarding transitions. A reusable extender remains conditional on
finding a safe intervention point; no extender or interlining fix exists yet.

## Evidence and reproducibility

- Executable: `game/installation/TransportFever3.exe`, 69,711,288 bytes.
- SHA-256: `5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
- Preferred image base: `0x140000000`; addresses below are preferred virtual
  addresses for this build, not runtime addresses or cross-version offsets.
- Ghidra 12.1.3 PUBLIC, Java 21, x86 64-bit Windows; project:
  `analysis/ghidra/TPF3Interlining`.
- Whole-program analysis reported an entry-point disassembly error and reached
  the 240-second timeout. Targeted function creation/decompilation succeeded.
  Cross-references are incomplete; decompiler switch/type warnings remain.
- Function boundaries were checked against PE exception-directory ranges.
  String loads included AVX instructions; a narrow LEA-only search missed them.
- Local evidence: `analysis/hook-investigation/` (ignored). Reproduction commands
  and export scripts are in [tools/ghidra](../tools/ghidra/README.md).

## Shipped scripting surface

Paths here are relative to `game/installation`.

`base/content/game_mechanics.zip::game_mechanics/towns/towns.script.tl`,
lines 371–393, handles `SimPersonSystem/OnModifyPathfinding`, modifies the map
for towns with active marketing, and returns the modifier. The declared local
record contains only `town2lineCostFactor : {Engine.Entity : number}`.

`base/tealdef/scripts/gamescript.d.tl` provides event subscriptions and a
return-bearing `handleEvent`. No subscription priority is exposed there.
`api/tealdef/api/engine.d.tl` describes engine access as read-only;
`SimEntityAtTerminal` contains concrete line and stop fields. Pathfinding types
in `api/tealdef/api/type.d.tl` describe mode costs and concrete line sections,
but the inspected declarations offer no command to replace a waiting person's
journey and update boarding state atomically.

`OnArriveAtStop`, `OnStartedLineUsage`, and `OnCompletedLineUsage` provide useful
observations. A bundled celebrations handler explicitly notes that
`OnStartedLineUsage` occurs before the person is in the vehicle. Treat it as a
line-use notification, not proof of boarding or a demonstrated override hook.

## Native callback flow

Function roles below are interpretations of the inspected code, not exported
symbol names.

| Address | Observed role |
| --- | --- |
| `0x14372c428` | `OnModifyPathfinding` string, not a function |
| `0x140c390e0` | Builds the callback payload, dispatches, reads back a modifier |
| `0x140c39e60` | Serializes the town/entity-to-number map |
| `0x140c35f60` | Decodes only `town2lineCostFactor` from a callback result |
| `0x140c362a0`, `0x140c360c0` | Map lookup and native map conversion |
| `0x140f43c40`, `0x140aae550` | Return-aware dispatch and subscriber iteration |
| `0x140f45430` | Successful non-nil script results enter result processing |
| `0x1403315e0` | Callback-result copy reducer; no explicit per-key merge found |
| `0x1404cc6a0` | Moves the resulting native map into the modifier |
| `0x140c3b500`, `0x140b34a00` | Installs the callback on a signal at system offset `0x170` |

The bridge creates one top-level field, `town2lineCostFactor`, and reads back
only that field. It passes no passenger, destination, eligible-line list, or
journey setter. With no returned override it copies the original modifier.
The decoder starts with an empty map: returning an incomplete replacement may
discard adjustments. Exact runtime consequences remain untested.

The dispatcher appears to propagate non-nil results sequentially through a copy
reducer. We have not established mod ordering, full exception behavior, or
cross-handler mutation visibility. A passive observer should leave the supplied
table untouched and return `nil`; the probe follows that rule.

## Runtime gate

The [probe and procedure](../probes/pathfinding/README.md) are ready for an
initial load test. Offline tests cover observer neutrality and bounded logging;
they do not verify resource loading, engine dispatch, cadence, or gameplay.

Still required: successful initialization, real payloads and callback timing,
baseline boarding observations, and multi-mod return/error behavior before any
weight-changing script. Town-map scope is established statically; how often
the engine requests it and when resulting costs affect existing journeys remain
open. Absence of logs without an initialization marker proves nothing about
hook availability.
