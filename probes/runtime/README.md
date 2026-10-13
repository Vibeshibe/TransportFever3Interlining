# Runtime capability probe

This is an ordinary Lua diagnostic mod, not a functioning interlining release.
It checks whether the game exposes the facilities needed by a shared native
hook dependency. It invokes no module loaders, commands or passenger mutations.

Copy `mod.json`, `_content.json`, `_metadata/` and `content/` into
`<game>/mods/tpf3_interlining_runtime_probe_1/`. Enable **TPF3 Interlining —
Runtime Capabilities** when loading a disposable save. Run briefly so the update
callback executes. Search `crash_dump/stdout.txt` for `[TPF3RuntimeProbe]`.
Each startup phase logs once. Removal requires only disabling/removing this mod.

A function being present proves availability only, not that arbitrary modules
or passenger changes are supported. A missing loader rules out that particular
entry point; it is not proof against every possible scripting approach.
