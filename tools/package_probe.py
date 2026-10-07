#!/usr/bin/env python3
"""Package the passive probe without installing it or including local tests."""

import json
from pathlib import Path
from zipfile import ZIP_DEFLATED, ZipFile


def main():
    root = Path(__file__).resolve().parents[1]
    source = root / "probes/pathfinding"
    manifest = json.loads((source / "mod.json").read_text())
    content = json.loads((source / "_content.json").read_text())
    json.loads((source / "_metadata/modinfo.json").read_text())
    mod_id = manifest["modId"]
    if mod_id != "tpf3_interlining_probe_1":
        raise ValueError("Unexpected probe mod ID")
    files = [Path("mod.json"), Path("_content.json"), Path("_metadata/modinfo.json")]
    for name in content["files"]:
        relative = Path(name)
        if relative.is_absolute() or ".." in relative.parts:
            raise ValueError(f"Invalid resource path: {name}")
        files.append(Path("content") / relative)
    for relative in files:
        if not (source / relative).is_file():
            raise FileNotFoundError(source / relative)
    destination = root / "analysis/packages" / f"{mod_id}.zip"
    destination.parent.mkdir(parents=True, exist_ok=True)
    with ZipFile(destination, "w", ZIP_DEFLATED) as archive:
        for relative in files:
            archive.write(source / relative, (Path(mod_id) / relative).as_posix())
    with ZipFile(destination) as archive:
        if archive.testzip() is not None:
            raise RuntimeError("Package CRC validation failed")
    print(f"Packaged {len(files)} runtime files: {destination}")


if __name__ == "__main__":
    main()
