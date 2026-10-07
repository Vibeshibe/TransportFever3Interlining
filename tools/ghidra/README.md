# Targeted Ghidra exports

Run from the repository root. These scripts write local evidence under ignored
`analysis/`; do not commit game decompilation or the Ghidra database.

The existing project is `analysis/ghidra/TPF3Interlining`, containing
`TransportFever3.exe`. Whole-program analysis was limited to four CPUs and 240
seconds and did not complete (entry-point disassembly error, then timeout).
Missing cross-references must not be interpreted as absent callers.

`ExportHookEvidence.java` takes an output path and string addresses; it reports
references, containing functions, and decompilation where available.
`ExportFunctions.java` takes an output path and known function addresses; it
disassembles/creates missing functions and exports callers and decompilation.
Use independently checked function starts, such as PE exception-table entries.
Both scripts have compiled and run with the supplied Ghidra 12.1.3 distribution.

Example against the existing project:

```sh
rtk proxy env GHIDRA_HEADLESS_MAXMEM=4G python3 tools/ghidra_headless.py \
  analysis/ghidra TPF3Interlining -process TransportFever3.exe -noanalysis \
  -scriptPath tools/ghidra \
  -postScript ExportFunctions.java analysis/hook-investigation/converters.txt \
  0x140c35f60 0x140c390e0 \
  -log analysis/hook-investigation/targeted.log
```

For a fresh import, create the output directories, replace `-process ...
-noanalysis` with `-import game/installation/TransportFever3.exe -max-cpu 4
-analysisTimeoutPerFile 240`, and retain the script arguments. Do not overwrite
the existing project to reproduce an import. The launcher redirects settings
and cache to `tools/runtime/`; MCP is unnecessary for these exports.

Verify the executable hash before reusing addresses. See the
[capability assessment](../../docs/scripting-hook-investigation.md) for the
fingerprint, function roles, limitations, and remaining runtime checks.
