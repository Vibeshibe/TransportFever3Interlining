# Repository Guidelines

## Project Structure & Module Organization

This directory is a Transport Fever 3 installation with editable scripts and assets, rather than a complete engine source repository.

- `base/content/`: core Lua (`.lua`), Teal (`.tl`), rendering shaders, and packaged asset archives (`.zip`).
- `api/tealdef/` and `base/tealdef/`: Teal type declarations (`.d.tl`).
- `mods/release/`: bundled mods and campaigns, with `mod.json`, content, metadata, and sometimes savegames.
- `dlcs/`: downloadable content packages.
- `vscode-template/`: editor extension recommendations and Teal configuration.
- `model_editor/`: model editor executable and plugins.

Keep changes scoped to the relevant script or content package. Avoid incidental edits to shipped executables, DLLs, and savegames.

## Build, Test, and Development Commands

No source build command, package-manager workflow, or automated test runner was found.

- On Windows, run `.\TransportFever3.exe` from the installation directory to validate changes in the game.
- Run `.\ModelEditor.bat` on Windows to launch the model editor with its plugin directory on `PATH`.
- Use `vscode-template/tlconfig.lua` as a configuration reference; replace `<tf3_install_folder>` placeholders with the actual installation path and include custom mod declarations as needed.

## Coding Style & Naming Conventions

Follow neighboring files: Lua and Teal scripts commonly use tabs; JSON manifests use four spaces. Preserve existing line endings and avoid unrelated reformatting. Use descriptive snake_case filenames, retaining established suffixes such as `.script.tl` and `.d.tl`. Preserve resource paths, mod IDs, and namespaced references when editing manifests. No repository-wide formatter or linter configuration was identified.

## Testing Guidelines

No automated framework or coverage threshold was found. Validate affected mods or campaigns in-game, exercise the changed behavior, and check that referenced assets load. For model or texture changes, inspect the result in the model editor and game. Record the scenario, reproduction steps, and observed outcome; use disposable saves for validation.

## Commit & Pull Request Guidelines

Git history is unavailable here, so no existing commit convention can be established. Use concise imperative messages, such as `Fix sandbox configuration initialization`. Keep each change focused. Pull requests should describe affected paths, expected behavior, and manual validation; link relevant issues and include screenshots for visual changes.
