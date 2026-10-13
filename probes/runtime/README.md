# Runtime capability probe

This is a diagnostic mod, not a functioning interlining release. Revision 1
inventoried capabilities; revision 2 additionally tries the remaining Lua native
module searchers with an inert Linux shared library. Its entry function only
prints a marker and returns. It creates no threads, installs no hooks and invokes
no game API. The script restores the previous `package.cpath` after the check.

Build the library into ignored output:

```sh
rtk proxy cc -std=c11 -O2 -Wall -Wextra -Werror -fPIC -shared \
  probes/runtime/native/handshake.c \
  -o analysis/runtime-loading/native/tpf3_runtime_handshake.so
```

Copy `mod.json`, `_content.json`, `_metadata/` and `content/` into
`<game>/mods/tpf3_interlining_runtime_probe_1/`, and the library into its
`content/native/` subdirectory. Enable **TPF3 Interlining — Runtime Capabilities**
on a disposable save; run briefly. Search `crash_dump/stdout.txt` for
`[TPF3RuntimeProbe]`. Output is once per phase per Lua state; the game creates
several states. Restart before replacing the library once it has loaded.

The probe tries a resolved resource path and the local installation's relative
mod path. A successful local load does not establish mod.io path resolution,
packaging acceptance, game-hook support, cross-platform support or save lifecycle
handling. A missing loader does not prove all native loading paths absent.

Revision 1 package is historical and contains only Lua/JSON. Revision 2's library
is a development experiment and has not been published to mod.io.
