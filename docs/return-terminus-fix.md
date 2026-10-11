# Return-to-first-terminus routing — 0.10.1

## Problem and change

The user reported S3 passengers remaining at Wimborne when S1 departed toward
Faringdon at15:36 on2026-10-09. The saved baseline places Faringdon first:
S3 returns from stop6 to0 and S1 from5 to0. Version0.10.0 excluded non-increasing
stop indices and replacement traversal beyond the last list entry. This blocks
an ordinary return to the first terminus. Individual screenshot passengers were
not identified in the capture; the route data and policy establish the exclusion
independently of discovery.

Version0.10.1 calculates forward distance around each native stop list and reads
successive stops modulo that line's own stop count. Either or both routes may
cross the boundary, including lines of different lengths. Actual native indices,
including alighting index0, are retained in requests and onboard checks.
No save, line name or entity ID is hard-coded.

The one-to-four-interval limit, ordered-subset rule, original endpoint station,
final destination, platform/permission/capacity checks and deferred validation
remain. Repeated stations within the original leg, complete circuits, detours,
excessive distance and invalid indices remain excluded. ABI8 and the engine-owned
mutation operation are unchanged.

## Validation and package

The warnings-as-errors build and all13 native invocations passed on2026-10-09.
Recovery on2026-10-11 verified report executable hashes against the binaries;
CMake reported no work required. Tests cover either/both routes wrapping,
different list lengths, one-to-four intervals, express skips, original endpoint
and index0 preservation, repeated-station rejection and consumer boarding.
Existing lifecycle/capacity/ABI regressions pass; worst query allowance63/64.

Prepared ignored build: `analysis/native-bridge/build-v10b`.
Package: `TPF3Interlining-0.10.1-trial.zip`, containing DLL, loader, report,
instructions and checksum/source manifest. Limits remain32 attempts,4 detailed
journey samples and18000 callbacks. The fix has not been loaded or tested live.

- DLL SHA256: `c26352cba241b8c4bcbc2ad238944caa4e7e3e1670057af1ffb5e673f44f94e8`.
- Report SHA256: `140cea8bd1766cae41a87c5e922188ed70146a919316cb5c68c5a006f0bac4ae`.

## Recovered 0.10.0 evidence

The prior log ends at4800 without an automatic summary or stop marker. It records
12 successful changes and both-world boardings, five across boarding platforms.
Four sampled original destinations were reached, including105864 following an
S3→S2 switch with different boarding and alighting platforms. Four sampled
journeys remain unresolved; four boardings were deliberately unsampled. No errors
are logged. Do not count unresolved or unsampled passengers as arrivals.

Ignored `build-v10a/recovered-audit.json` records this partial evidence. Log SHA:
`e1734ecf1643806831f0bad76e58fb8c18db3cfb8bfda141e69c5a95a9f1f1e3`.
The cause of recording ending is unknown. No game process was found on2026-10-11;
old process IDs and world addresses must not be reused.

## Next integrated trial

Open a disposable `TPF3Interlining_before_multi_passenger_01` with the existing
platform layout and leave it paused. Verify the fresh process, actual save,
game fingerprint and new DLL using the [native guide](../native/README.md).
Restart first if an older bridge is loaded. Then unpause normally without line
or vehicle edits. Observe Wimborne→Faringdon: original6→0 and replacement5→0
should be eligible when the remaining checks pass. Confirm both-world boarding
and sample the original destination. These indices describe the fixture only.
No additional read-only replay is required before this trial.
