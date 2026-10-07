# Passive pathfinding probe

Experimental diagnostics for task 003. This is not an interlining fix. The
handler leaves routing payloads untouched and returns `nil`; it writes only its
own counters and logs. Offline checks pass. Revision 1 loaded and dispatched all
four callbacks in [Test 1](../../docs/test-run-2026-10-07.md). Revision 2 adds
arrival snapshots; it is installed but its new engine queries need a game run.

## Prepare and install

From the repository root:

```sh
rtk proxy python3 tools/package_probe.py
```

This creates `analysis/packages/tpf3_interlining_probe_1.zip`, containing the
mod directory and its five runtime files. The verified installation from Test 1
is `<game installation>/mods/tpf3_interlining_probe_1/mod.json`, here:

```text
/home/vibeshibe/Games/Transport Fever 3/mods/tpf3_interlining_probe_1/mod.json
```

The installed copy has been updated to revision 2; no manual reinstall is needed.
The two replaced files are backed up under ignored
`analysis/probe-install-backups/revision1-to-2/`.
For a fresh installation, extract the archive into the
installation's `mods/` directory. The previously suggested Proton user
`local/mods/` location has not been validated. Packaging does not install or
activate anything. Enable **TPF3 Interlining — Pathfinding Probe** for a disposable
baseline copy and keep the preserved baseline untouched.

## First run: about five minutes

For revision 2, restart the game and load a disposable baseline/Test1 copy.
Expect `revision=2; passive mode; arrival snapshots enabled`. Upgrading an
existing probe save resets diagnostic counters once; its subscription marker
might not repeat because subscriptions already exist.

1. Load the disposable copy and run briefly. Check for
   `[TPF3InterliningProbe] subscribed; passive mode` in the log.
2. Run at normal speed through several S1/S2/S3 arrivals at a shared stop.
   Record the stop, approximate elapsed time, and any ignored vehicles with
   spare seats. Do not change line settings during this initial run.
3. Pause for roughly 15 seconds, resume, and record that boundary. After several
   more arrivals, finish the run. Save only under a new experiment name if needed.
4. Provide the log path and observations. The currently detected log is the same
   Proton prefix's `drive_c/users/Public/Documents/Steam/RUNE/3493540/local/`
   directory's `crash_dump/stdout.txt`. Timestamps use UTC (two hours behind
   Europe/Berlin during Test 1). Preserve a copy before
   another game session can replace it; it can go under ignored `analysis/`.

Expected log events: `OnModifyPathfinding`, `OnArriveAtStop`,
`OnStartedLineUsage`, and `OnCompletedLineUsage`. Each includes a count and,
when available, game time/tick/update counters. Only the first 20 and every
100th event of each type are logged; gaps afterward are intentional. Counters
persist in probe state. Line-use start is not a confirmed boarding event.

Revision 2 also logs `ArrivalSnapshot`, `QueueSnapshot`, and `WaitingPerson`
records for the first 60 arrivals and every 100th arrival thereafter. They share
an arrival counter and vehicle ID for correlation. Snapshots include line/station
names, configured and actual terminal indices, capacity/occupancy, loading
configuration, and sampled passengers' planned boarding/alighting stops.
Snapshots are taken at the arrival notification, not after loading completes.

Traversal is limited to 16 line-stop queues, 8 passengers per queue, and 32
passengers total per arrival. Limit messages identify truncation; API queries
can still allocate full returned lists. No global passenger scan is used. Queue
order can bias the sample. An alighting station is not necessarily the final
destination, and matching stations alone does not prove boarding eligibility.
`SnapshotError` reports a failed diagnostic query without overriding routing.

If initialization appears but a particular event does not, report which events
did appear. If initialization is missing, retain loading errors: that is a
probe-integration problem until shown otherwise. Stop the experiment on script
errors and retain the log. Disable the probe for ordinary play.

## Offline checks

```sh
rtk proxy lua probes/pathfinding/test_probe.lua
rtk proxy lua probes/pathfinding/test_snapshots.lua
```

Checks subscriptions, nil returns, unchanged routing data, sampling, event
summaries, and unavailable timestamps. Snapshot tests cover zero-based indices,
nonzero passenger cargo IDs, terminal differences, stale entities, errors,
sampling bounds, and revision migration. These use stubs, not the engine.
