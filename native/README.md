# Shared native bridge — experimental read-only pilot

The bridge provides one centrally owned terminal-update interception and a small
versioned C API. **It cannot change passenger routes or components.** The Lua
policy in `src/interlining/` remains separate and is not connected to this DLL.
This is a development loader, not an automatically installed game mod.

## Build and isolated tests

On Linux x86-64, with Python 3.12+, CMake, Ninja, and Wine:

```sh
rtk proxy python3 tools/build_native_bridge.py --fetch-toolchain
rtk proxy env WINEPREFIX="$PWD/tools/runtime/bridge-test" WINEDEBUG=-all \
  wine analysis/native-bridge/build/bridge_tests.exe
```

Downloads are explicit. The helper pins LLVM-MinGW 20250910 (LLVM 21.1.1), verifies
the release archive SHA-256, and extracts only into ignored `tools/vendor/`.
Subsequent builds omit `--fetch-toolchain`. `--toolchain PATH` selects another
installed LLVM-MinGW directory. Builds and runtime logs stay under ignored
`analysis/`. Do not overwrite a DLL currently loaded in the game; use a new build
directory and restart the game before testing a changed bridge.

Outputs: `tpf_shared_bridge.dll`, `bridge_loader.exe`, `bridge_tests.exe`, and
`bridge_fixture.exe`. The fixture runs for 60 seconds and prints its Windows PID.
Loading the production DLL there must return status `9` (unsupported build).
There is no production switch that bypasses executable verification.

## Explicit loading under Windows/Proton

Use a disposable save. Run the loader in the **same Proton version and prefix**
as the game; Linux and Windows PIDs differ. The loader supports:

```text
bridge_loader.exe --list
bridge_loader.exe WINDOWS_PID Z:\absolute\path\tpf_shared_bridge.dll
bridge_loader.exe WINDOWS_PID Z:\absolute\path\tpf_shared_bridge.dll --stop
```

Use an existing absolute Windows DLL path under 260 characters. Loading is two
steps: `LoadLibraryW`, then `tfx_start` on a separate remote thread after library
initialization. Loader operations time out after 30 seconds; they never terminate
a timed-out target thread or unload possibly running code. An argument allocation
may remain until process exit on such a timeout. No proxy DLL, launch override,
game-file replacement, or persistence is installed.

The bridge writes `tpf-bridge-WINDOWS_PID-UPTIME.log` beside the DLL. Start returns
zero only after verifying the executable, live entry bytes, table layout, and
hook ownership. Run the simulation to collect up to 400 accepted invocations;
the first eight and final invocation are logged. This is not a game-tick count:
two engine worlds can alternate. After the limit, the wrapper forwards without
observation. `--stop` also disables future observations; an in-flight callback
may finish. **Restart the game to remove the wrapper or retry initialization.**
The DLL is pinned until process exit. It does not save persistent state.

## Client contract

[`include/tpf_bridge.h`](include/tpf_bridge.h) is the ABI. Resolve `tfx_get_api`
from the loaded DLL; request ABI 1 with `tfx_api.struct_size = sizeof(tfx_api)`.
The only capability is `TFX_CAP_TERMINAL_OBSERVATION`. Registration copies a
`tfx_client` containing an ASCII ID, callback and caller-owned user-data pointer.
Keep the callback module and user data alive until unregistration succeeds.
Example callback:

```c
static tfx_api bridge = {.struct_size = sizeof(tfx_api)};
static void before_selection(tfx_context context, void *user_data) {
    (void)user_data;
    tfx_observation sample = {.struct_size = sizeof(sample)};
    if (bridge.read_observation(context, &sample) != TFX_OK) return;
    /* Consume a copied invocation sequence and delta-time bits. */
}
```

Clients run lexically by ID, independent of registration order. Duplicate IDs,
more than 32 clients, and registration changes during dispatch are rejected.
Contexts expire at the end of **each client's** callback. A previous client's
token, another thread, or nested dispatch cannot access the current observation.
No world/component/function pointer crosses the API. Callbacks must return
promptly and normally: this native interface does not contain malicious code,
access violations, exceptions, or hung clients. There is no Lua binding yet.

## Interception and compatibility

Pinned executable SHA-256:
`5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35`.
The terminal-system table is at RVA `0x3702758`; slot `+0x58` at `0x37027b0`
points to Update at `0xb0db00`. Static RTTI identifies
`ecs::SimEntityAtTerminalSystem`. Adjacent lifecycle entries and 16 Update bytes
are checked in the relocated image. An atomic expected-pointer exchange changes
the slot; original page protection is restored. A named per-process ownership
handle rejects another cooperating bridge. Independent patchers remain outside
this agreement and can still conflict.

[`entry.S`](entry.S) saves volatile integer/XMM registers and the original stack,
calls the observer, then tail-jumps to the original function. It has Windows x64
unwind metadata and does not relocate engine instructions. The observer checks
the system table, update-data pointer, thread, and modification-window flags.
The original engine call still executes on rejected observations. Only a null
window and enabled nested/post flag are accepted; no engine mutation is called.

Registry tests exercise two consumers, expired tokens, thread/reentrancy guards,
limits, and duplicate IDs. The slot/assembly test checks ownership refusal,
restored page protection, integer/float/stack arguments, original return value,
and exactly one original call. These tests do not prove arbitrary native-client
safety, cross-line migration, save compatibility, or third-party mod support.

Implementation references: Microsoft's [x64 calling convention](https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention),
[atomic pointer exchange](https://learn.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-interlockedcompareexchangepointer),
and [DLL initialization guidance](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-best-practices);
official [LLVM-MinGW release](https://github.com/mstorsjo/llvm-mingw/releases/tag/20250910).
