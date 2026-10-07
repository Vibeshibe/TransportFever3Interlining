# Native bridge validation — 2026-10-07

## Implemented scope

The [read-only bridge](../native/README.md) now builds and loads under the game's
Proton runtime. It owns terminal Update interception separately from passenger
policy. No route/component mutation or Lua binding is implemented.

Static inspection found the sole `.rdata` pointer to Update at RVA `0x37027b0`.
The table starts at `0x3702758`; Update occupies slot `+0x58`, matching the
previously observed dispatcher. Its complete-object locator at RVA `0x390c988`
references type descriptor `0x3d948c8`, named
`.?AVSimEntityAtTerminalSystem@ecs@@`. Neighboring entries match the terminal
lifecycle callbacks. This permits atomic slot replacement without moving engine
instructions. It is not a general detour library.

## Build and isolated validation

- Pinned official LLVM-MinGW 20250910/LLVM 21.1.1 archive, verified SHA-256
  `f83556c9ffa4d4291fadea1a0776c1383332dacdf4d7fbdf974c2928cb32c6f7`.
  Toolchain is local and ignored, not a system installation.
- CMake/Ninja built the DLL, explicit loader, fixture and tests with
  `-Wall -Wextra -Werror`. Only Windows system/UCRT DLL imports were found;
  no separately deployed compiler-runtime DLL is required by these binaries.
- `bridge_tests.exe` passed in an isolated Wine prefix. It checks two consumers
  registered in reverse order, duplicate IDs, malformed inputs, per-client token
  expiry, cross-thread access, registration/reentrancy during dispatch, client
  limits, and unregistration. It also replaces a protected fixture slot, rejects
  ownership conflicts, and verifies restored protection, float/integer/stack
  arguments, original return value, and one original invocation per call.
- `llvm-readobj --unwind` confirmed `tfx_terminal_entry` has a seven-byte prolog
  and a 200-byte stack allocation described in Windows unwind metadata.
- The explicit loader loaded the production DLL into `bridge_fixture.exe`.
  `tfx_start` returned `9` (unsupported build); the log records fingerprint
  refusal. `tfx_stop` returned zero. There is no production build-check bypass.
- `tools/build_native_bridge.py` ran successfully with the installed toolchain;
  its Python syntax check passed. The existing observer and Lua policy were not
  changed or unnecessarily retested.

## In-game read-only capture

The still-running disposable game was Linux PID `79540`, Windows PID `308`.
Used its exact GE-Proton11-5 runtime and actual Wine prefix ending in
`Transport Fever 3/pfx/`; did not mix system Wine into that prefix.

The loader returned zero after executable SHA-256, live entry/table and ownership
checks. It changed only the in-memory virtual slot. Initially no calls arrived;
reading the bridge's own counters confirmed zero accepted and rejected calls.
The user then unpaused at normal speed.

The bridge accepted **400 invocations**, then disabled observation automatically.
The log samples invocations 1–8 and 400; all report status zero and zero rejected
boundaries. The first eight alternate worlds `0x2d75b860` / `0x2d75c520`; the final
sample uses `0x2d75c520`. Thread `592` is the Windows simulation thread ID.
Logged delta bits `0x3e4ccccd` represent approximately 0.2 seconds. Thread and
modification-window guards were enforced throughout accepted dispatches.
There were no external native clients registered in the game during this pilot;
the two-consumer validation is from the isolated test process.

Explicit `--stop` also returned zero after completion. No debugger was attached
for this capture. No game file, save, installed Lua probe, passenger component or
queue was modified by the bridge. The DLL remains pinned and its wrapper forwards
until game exit; closing/reopening removes the in-memory change. It has no autoload.

Ignored artifacts in `analysis/native-bridge/build/`:

| Artifact | SHA-256 |
| --- | --- |
| `tpf_shared_bridge.dll` | `ee6dfe1e756b0c186deed2a3215b9bbf7781177c35f287fc42439594890cd502` |
| `bridge_tests.exe` | `f6eca34ea7f547de0a200b0beca420b1385c0fcb424e23865da20f3e81a9d3ed` |
| `bridge_loader.exe` | `8af4e4ef375fe4d362784a52e8fa61f8f1bce30a98ea209dded5623fe9bc94e8` |
| `tpf-bridge-308-22280336.log` | `ffa75b18f01e2d832ba896a5a07c8e511c96090ddd7869df7014b150f3697db4` |

## Remaining gates

This validates the selected interception and forwarding pilot, not passenger
interlining. Implement copied live waiting-leg/route/capacity queries next. Keep
mutation unavailable until expected-state, route occurrence, destination,
terminal, permissions, reservations and replication checks are implemented.
Then test one designated passenger through engine-owned remove/add operations.
Do not infer save/reload, native Windows, exception containment, third-party
compatibility, or varied preferences from this successful Proton capture.
