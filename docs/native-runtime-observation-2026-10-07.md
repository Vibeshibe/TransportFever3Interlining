# Native observation — 2026-10-07

The user loaded a disposable interlining test save and initially left it paused,
then unpaused at normal speed. No passenger-state writes or injected engine calls
were performed. The installed Lua probe and game files were not changed.

## Setup and evidence

- Game process: `74799`, Proton process name `Main Thread`.
- Executable SHA-256:
  `5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
- Runtime module base: `0x6ffff9230000`; preferred base: `0x140000000`.
- Four hardware breakpoints; source and live entry bytes verified before use.
- Raw captures, ignored: `analysis/native-boarding/runtime-trace-01.jsonl`
  and `runtime-trace-02.jsonl`; stack snapshot: `runtime-threads.txt`.
- Successful capture SHA-256:
  `71817665ad7812515b419897fb4797a89a674c5d7a0c85ca258d43cfb469121e`.

The first capture had no events because simulation was still paused. It was
interrupted and detached cleanly. The observer was revised to start its
120-second limit at the first hit, then reattached after the user unpaused.
The second capture ended normally at the hit budget and detached. No debugger
was left attached.

## Recorded sequence

All records concern person `71583`, with line S3=`97743`, stops 3→4,
vehicle `97914`. Thread ID was `75558` (`Simulation Thre`). Times below are
debugger elapsed seconds, not game timestamps. Addresses are preferred return
addresses reconstructed from the runtime module base.

| Elapsed | Operation | World | Caller | Details |
| --- | --- | --- | --- | --- |
| 3.162 | Waiting add | `0x2d3d6530` | `0x140b36be6` | Vehicle `-1` |
| 3.331 | Waiting removal/add | `0x2d3d71f0` | `0x140b36880` / `0x140b38a7d` | Vehicle `97914`; same arrival-time bytes |
| 3.497 | Waiting removal | `0x2d3d6530` | `0x142bb455e` | Replica replay |
| 7.148 | Waiting removal, boarding entry | `0x2d3d6530` | `0x140b36880` / `0x140b34718` | Same line, stops, vehicle |
| 7.149 | Onboard add | `0x2d3d6530` | `0x140b37861` | Common transition |
| 7.311–7.312 | Waiting removal, onboard add | `0x2d3d71f0` | `0x142bb455e` / `0x142bb44fe` | Replica replay |

There were two waiting additions, four waiting removals, one boarding entry,
and two onboard additions: nine records from 5,001 total breakpoint hits in
11.104 seconds. Generic component removal generates many unrelated hits.

Every sampled world had a non-null `m_betweenChanges` and a zero notification
flag. Direct operations had the nested/post flag enabled; observed replay
removals and onboard addition had it disabled. Static `Replicate` and replay
analysis explains the repeated operations in distinct worlds. This is not
evidence of duplicate passengers.

## Conclusions and limits

The component layouts, Windows x64 argument decoding, relocation checks, and
ordinary boarding call path now have runtime evidence. Breakpoints are at
function entries: a record alone does not prove completion of that call.
The trace does not establish interlining, cross-line queue migration, transfer
correctness, seat allocation completion, or an approved mutation boundary.
No performance conclusion can be drawn from debugger elapsed times.

Next capture should target terminal-update boundaries and modification/replay
context rather than the generic removal routine. Keep engine-world pointers
scoped to each observed callback; both instances participate in simulation.
