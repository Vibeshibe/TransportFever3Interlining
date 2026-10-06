#!/usr/bin/env python3
"""Launch the local Ghidra headless analyzer with project-local settings/cache."""

import os
from pathlib import Path
import subprocess
import sys


def main():
    project = Path(__file__).resolve().parents[1]
    ghidra = project / "tools/downloads/ghidra_12.1.3_PUBLIC"
    launcher = ghidra / "support/analyzeHeadless"
    if not launcher.is_file():
        sys.exit(f"Ghidra launcher missing: {launcher}")
    environment = os.environ.copy()
    for key, directory in (("XDG_CONFIG_HOME", "config"), ("XDG_CACHE_HOME", "cache")):
        path = project / "tools/runtime" / directory
        path.mkdir(parents=True, exist_ok=True)
        environment[key] = str(path)
    return subprocess.call([str(launcher), *sys.argv[1:]], env=environment)


if __name__ == "__main__":
    sys.exit(main())
