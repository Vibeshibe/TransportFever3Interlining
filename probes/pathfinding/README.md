# Passive pathfinding probe

Experimental diagnostics for task 003. This is not an interlining fix. The
handler leaves routing payloads untouched and returns `nil`; it writes only its
own counters and logs. Offline checks pass; in-game loading is unverified.

## Prepare and install

From the repository root:

```sh
rtk proxy python3 tools/package_probe.py
```

This creates `analysis/packages/tpf3_interlining_probe_1.zip`, containing the
mod directory and its five runtime files. Extract that directory into the
game's user `local/mods/` directory. The currently detected parent directory is:

```text
/home/vibeshibe/Games/Heroic/Prefixes/default/Transport Fever 3/drive_c/users/Public/Documents/Steam/RUNE/3493540/local/mods/
```

The resulting path must end in
`local/mods/tpf3_interlining_probe_1/mod.json`. Packaging does not install or
activate the mod. In the game's mod selection for a disposable copy of
`TPF3Interlining_Baseline`, enable **TPF3 Interlining — Pathfinding Probe**.
Keep the preserved baseline untouched. If the mod is missing from the selection,
report that first; its discovery/loading procedure still needs validation.

## First run: about five minutes

1. Load the disposable copy and run briefly. Check for
   `[TPF3InterliningProbe] subscribed; passive mode` in the log.
2. Run at normal speed through several S1/S2/S3 arrivals at a shared stop.
   Record the stop, approximate elapsed time, and any ignored vehicles with
   spare seats. Do not change line settings during this initial run.
3. Pause for roughly 15 seconds, resume, and record that boundary. After several
   more arrivals, finish the run. Save only under a new experiment name if needed.
4. Provide the log path and observations. The currently detected log is the same
   user `local/` directory's `crash_dump/stdout.txt`. Preserve a copy before
   another game session can replace it; it can go under ignored `analysis/`.

Expected log events: `OnModifyPathfinding`, `OnArriveAtStop`,
`OnStartedLineUsage`, and `OnCompletedLineUsage`. Each includes a count and,
when available, game time/tick/update counters. Only the first 20 and every
100th event of each type are logged; gaps afterward are intentional. Counters
persist in probe state. Line-use start is not a confirmed boarding event.

If initialization appears but a particular event does not, report which events
did appear. If initialization is missing, retain loading errors: that is a
probe-integration problem until shown otherwise. Stop the experiment on script
errors and retain the log. Disable the probe for ordinary play.

## Offline checks

```sh
rtk proxy lua probes/pathfinding/test_probe.lua
```

Checks subscriptions, nil returns, unchanged routing data, sampling, event
summaries, and unavailable timestamps. These use stubs, not the engine.
