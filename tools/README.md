# Research tools

## Executable inspection

Run from the project root:

```sh
rtk proxy python3 tools/inspect_game.py > analysis/executable-report.json
```

This reads the executable, computes its SHA-256, and finds exact diagnostic
strings with file offsets, RVAs, and preferred virtual addresses. These are
string addresses; follow cross-references to locate candidate functions.
It does not modify the executable or run it. Python's standard library suffices.
Use `--exe /path/to/TransportFever3.exe` to inspect another build.

## Ghidra

The user supplied an extracted Ghidra 12.1.3 PUBLIC distribution in
`tools/downloads/ghidra_12.1.3_PUBLIC`. Its local application properties report
revision `8b4c91d4d5bd1549622bfbade0df199585b98365` and JDK 21 minimum.
The original ZIP was not supplied, so its published archive checksum has not
been verified against this extracted copy. Ghidra MCP remains uninstalled.

Use [Ghidra 12.1.3's official release](https://github.com/NationalSecurityAgency/ghidra/releases/tag/Ghidra_12.1.3_build)
as a pinned starting version. Its [application properties](https://github.com/NationalSecurityAgency/ghidra/blob/Ghidra_12.1.3_build/Ghidra/application.properties)
specify JDK 21 minimum, which is available locally. Launch the supplied copy with:

```sh
rtk proxy python3 tools/ghidra_headless.py
```

With no arguments, this prints headless usage and exits with status 1 because
no project arguments were supplied; this startup check passed. The wrapper forwards arguments
and keeps settings/cache in ignored `tools/runtime/` directories. Downloaded
tools remain outside Git. In the GUI, create a non-shared project under `analysis/` and import
`game/installation/TransportFever3.exe`. Record analysis settings and version.

If downloading a fresh copy later, use the official release ZIP named
`ghidra_12.1.3_PUBLIC_20260817.zip`; save it in `tools/downloads/` and verify it
with `rtk proxy sha256sum --check tools/ghidra.sha256` before extraction.

## Optional Ghidra MCP

Headless Ghidra scripts can perform the initial cross-reference analysis without
MCP. If interactive MCP control is needed, evaluate
[LaurieWired/GhidraMCP](https://github.com/LaurieWired/GhidraMCP), which provides a
Ghidra extension and a Python bridge. Check extension compatibility with the
installed Ghidra version before selecting a release or building it.

Keep its checkout under `tools/vendor/GhidraMCP` and Python dependencies in
`tools/.venv`. Follow upstream installation instructions to enable the extension;
then connect the bridge to its local Ghidra server. Record the exact upstream
commit, dependencies, and successful tool call before marking MCP operational.
No MCP client configuration has been changed by this setup.
