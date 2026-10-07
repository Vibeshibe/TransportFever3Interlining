# Native observation under Proton

This GDB observer reads waiting-component removal/addition and boarding entry
arguments for the executable pinned in `profile.json`. It uses four hardware
execution breakpoints, verifies the executable SHA-256 and live entry bytes,
and relocates addresses using the game's file-offset-zero mapping. It makes no
engine calls, writes no passenger state, and installs no mod or native extension.
Debugger stops affect timing; these captures are not performance benchmarks.

Run against a disposable save, initially paused. Locate the PID from the
`TransportFever3.exe` command line; Proton calls the process **Main Thread**,
so searching only process names misses it. Use a fresh GDB session:

```sh
rtk proxy gdb -nx -batch \
  -ex 'set debuginfod enabled off' \
  -ex 'set print thread-events off' \
  -ex 'attach GAME_PID' \
  -ex 'source tools/native_trace/gdb_trace.py' \
  -ex 'interlining-trace "game/installation/TransportFever3.exe" "analysis/native-boarding/runtime-trace.jsonl" 5000' \
  -ex 'detach'
```

Replace `GAME_PID` and choose a new output filename. The output must not exist.
Host debugger permissions may require execution outside the shell sandbox.
Do not change system-wide ptrace settings just to run this observer.

After **Native observer ready**, unpause at normal speed. Capture stops at
5,000 total breakpoint hits (including unrelated component removals), 400 logged
records, or the next breakpoint after 120 seconds from the first hit. While paused there may be no
hits: this is not an independent wall-clock watchdog. Interrupt GDB if necessary;
the command's cleanup removes its breakpoints, and `detach` resumes the process.
Unsupported signatures or exhausted hardware breakpoint slots abort the capture.

Records include the Linux thread ID, world pointer, caller return address,
line/stop/vehicle fields, and the engine's `m_betweenChanges` pointer,
`notificationInProcessDEBUG` byte and `m_allowPostAndNestedChangesHack` byte.
These flags are observed preconditions, not proof that arbitrary writes are safe.
`boarding_transition` and `onboard_add` are function **entries**, not confirmation
that a transition completed. Arrival time is preserved as raw bytes; decode its
numeric meaning only after confirming the native representation.

The profile is specific to one game build. Never regenerate its signatures to
bypass a mismatch without repeating function/ABI analysis. The GDB module uses
Python's standard library; byte decoders have standalone tests:

```sh
rtk proxy python3 -m unittest discover -s tests -p 'test_native_trace_decode.py'
```

## Terminal-update boundary capture

Use `interlining-trace EXE OUTPUT 160 boundary` to select four sites: terminal
Update entry, immediately after opening its commit scope, immediately after
closing that scope, and the return instruction. These include interior sites;
their fingerprints and stack offsets are specific to the pinned build.

The observer pairs calls by thread and entry stack, validates phase order and
world identity within each call, and discards the context at return. Attaching
mid-call skips that partial call. A capture may end at its budget mid-call;
the final `end` record is not evidence that the original call failed.

The first runtime captures covered 280 complete calls with only entry/return
records; no assignment scope was reached in those samples. Do not report the
two interior sites as runtime-validated. A full scope takes four hits; a call
that skips it takes two. Run both decoder and correlation tests with:

```sh
rtk proxy python3 -m unittest discover -s tests -p 'test_native_trace_*.py'
```
