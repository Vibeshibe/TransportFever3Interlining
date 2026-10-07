# Passenger and route queries — 2026-10-07

## Implementation

Bridge 0.2 adds ABI 2 while preserving ABI 1. Clients can read copied waiting-leg
and line-stop snapshots, or page through candidate waiting people. No engine
getter, component notification, queue mutation or passenger reassignment is called.
The bounded example consumer uses only the public API.

Private readers resolve five type indices in the current world's registry,
validate entity/component vectors, and copy either contiguous storage or shared
32-entry pages. Type indices and world pointers expire with the dispatch.
Missing, unreadable and malformed records have explicit failure statuses; failed
queries leave caller outputs unchanged. Every dispatch shares a 64-query,
8,192-read, 1 MiB budget. Scans inspect at most 1,024 masks and return at most
16 candidate IDs; callers must read those IDs again before using them.

## Static evidence

Targeted exports are ignored in `analysis/native-boarding/read-lookups.txt`,
`read-storage.txt`, and `read-layouts.txt`.

| Finding | Evidence |
| --- | --- |
| Type registry at world `+0x48`, 16-byte entries, stored index plus one | `0x140099970`, `0x1400a4cc0`, `0x1400a4d90` |
| Entity component lists at world `+0x90`, 8-byte type/index pairs | `0x1400a4b90` |
| Storage pointer vector at world `+0x78`; contiguous `+0x68`, shared pages `+0x80` | Typed component accessors below |
| Shared index tag `0x40000000`, 32 values/page, 16-byte page records | `0x140095a50`, `0x140095bc0`, `0x140095970`, `0x1400955b0`, `0x1402809d0` |
| Component strides: waiting 32, person 152, line 40, group 24, terminal-place 12 bytes | Same accessors; line accessor receives world `+0x48` |
| SimPerson destination at `+0x50`; waiting state/arrival/vehicle offsets | Prior boarding transitions and public Teal definitions |
| Line's stop vector at zero; stop stride `0x90`; stop configuration at `+0x50` | Terminal Update plus field registrations in `0x1418829c0` |
| Stop group/station/terminal, loading mode, load bit vector and maximum fractions | Same registrations and `api/tealdef/api/engine.d.tl` |

Read-only walking deliberately avoids the engine's asserting getters on absent
components. Bounds are conservative: unsupported layouts fail closed. The
executable fingerprint and Update/table checks remain mandatory at installation.

## Validation

Warning-clean LLVM-MinGW/CMake build in `analysis/native-bridge/build-v2`.
Both `bridge_tests.exe` and new `query_tests.exe` passed under the isolated Wine
prefix. Query fixtures exercise contiguous/shared storage, missing components,
malformed vectors/type mappings, unreadable memory, absent terminal-place data,
disabled loading, non-finite fractions, scan pagination/page limits, budgets,
unchanged failed outputs, and ABI 2 token/thread checks. ABI 1 and original-call
forwarding tests still pass.

The user restarted and loaded the disposable save. The new DLL loaded successfully
in Linux process `87399`, Windows process `308`, through GE-Proton11-5. After the
user resumed simulation, the capture reached **400 callbacks**, with **20 passenger
records (8 unique IDs)** and **40 stop records**. All sampled queries returned
zero; no scan/waiting-query error was logged. Boundary rejection count stayed zero.
Windows simulation thread was `596`; entry samples alternate worlds
`0x2eb2bca0` and `0x2eb2c960`.

Examples:

- Person `114323`: S3 (`97743`), stops 1→4, vehicle `-1`, destination `86134`.
  Boarding is group `97291`, station `97948`, terminal 1. This is a multi-stop leg
  and does not meet the current offline adjacent-segment policy.
- Person `87382`: S2 (`97742`), stops 0→1. Stop 1 is the same group/station,
  terminal 2. This matches the S2/S3 platform distinction documented in Test 2.
- S3 stop 2 resolves to group `97928`, station `97294`, terminal 0. Observed
  passenger-load flags are enabled and maximum fractions are 1.0; these are
  configuration values, not evidence of available seats.

Observer auto-disable completed. An optional subsequent `--stop` loader command
hung under Proton without returning a status; the cause is unresolved. Direct
read-only inspection of bridge counters confirmed `enabled=0`, `observed=400`,
`rejected=0`, `simulation_thread=596`. Terminated only that loader process, not
the game or an injected target thread. No debugger was attached to the game.
Attempting a loader-only backtrace was denied by OS ptrace permissions.
The DLL remains pinned, forwarding only, until game exit. Native loader timeouts
do not cover all Wine startup/API stalls; use an outer process timeout for future
manual loader invocations. No installed probe, save, game file or passenger state
was modified by this work.

Ignored artifacts in `analysis/native-bridge/build-v2/`:

| Artifact | SHA-256 |
| --- | --- |
| `tpf_shared_bridge.dll` | `6ac1cc139c0af1462189ebb9ed97c1c3e1d4b738e5f895d71bf64a95c97e255c` |
| `query_tests.exe` | `e2eefb1cb333bd30705834f4b23a97b72ba17fe669ab67d14c4ac97a98f2dde0` |
| `bridge_tests.exe` | `3ded08f59088cc292fcf9b06dc637e3373773cf63dfb8df554cf72a580efcb1f` |
| `tpf-bridge-308-23938409.log` | `d49c23c114ee1dfa1823b444d702302cd8bdb0e53eb9292b98ea98397b171cc2` |

## Remaining gates

Configured terminals are not actual waiting-place/arriving-vehicle compatibility.
Opaque terminal-place bytes preserve evidence but do not resolve platform access.
Vehicle capacity and reservations, actual boarding terminal, complete route
occurrence and destination/unloading checks remain outstanding. Scans across
callbacks/worlds are not one immutable snapshot, and the passenger log does not
identify each record's world. No mutation request API or eligibility decision is
exposed. Resolve those gates before the one-passenger remove/add experiment.
