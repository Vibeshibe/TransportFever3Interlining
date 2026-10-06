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

Installation is pending: downloads failed because this session cannot resolve
GitHub. Ghidra MCP is also not installed or connected.

Use [Ghidra 12.1.3's official release](https://github.com/NationalSecurityAgency/ghidra/releases/tag/Ghidra_12.1.3_build)
as a pinned starting version. Its [application properties](https://github.com/NationalSecurityAgency/ghidra/blob/Ghidra_12.1.3_build/Ghidra/application.properties)
specify JDK 21 minimum, which is available locally. The following installation
commands are prepared for a network-enabled shell; they have not completed here:

```sh
rtk proxy mkdir -p tools/downloads tools/vendor
rtk proxy curl --fail --location --connect-timeout 15 --output tools/downloads/ghidra_12.1.3_PUBLIC_20260817.zip https://github.com/NationalSecurityAgency/ghidra/releases/download/Ghidra_12.1.3_build/ghidra_12.1.3_PUBLIC_20260817.zip
rtk proxy sha256sum --check tools/ghidra.sha256
rtk proxy unzip -q tools/downloads/ghidra_12.1.3_PUBLIC_20260817.zip -d tools/vendor
rtk proxy tools/vendor/ghidra_12.1.3_PUBLIC/support/analyzeHeadless
```

Only extract if the checksum check succeeds. The final command should print
headless usage and establish that Ghidra starts. Downloaded tools remain outside
Git. In the GUI, create a non-shared project under `analysis/` and import
`game/installation/TransportFever3.exe`. Record analysis settings and version.

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
