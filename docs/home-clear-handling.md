# Bounded handling of cleared homes — 0.7.4

## Evidence and limits

The 0.7.3 continuation trial stopped on a home/identity mismatch after four
boardings. Its log did not preserve the triggering passenger or home value.
Later copied reads found120770 without SimPerson. The existing no-mutation
control reproduced that removal alongside redevelopment of home87062.

A diagnostic rerun selected40365 instead of120770 as its third passenger.
An external read-only monitor still followed120770: its passenger record vanished
in world2 at17:14:22.259833Z and world1 at17:14:22.361911Z, while87062 gained
Scaffold.120770 had not been switched in this rerun. The monitor did **not** catch
a transient cleared home. This supports a native lifecycle explanation but does
not prove the earlier guard's exact cause.

Targeted Ghidra export of PE-unwind-confirmed `0x140b33470` shows three paths
that test the first SimPerson destination for `-1` and then call entity removal
`0x142bb75f0`. A valid home instead permits native continuation/return-home logic.
This is static evidence for the pending state, not runtime attribution.

## Implementation

Version0.7.4 treats exactly `home=-1` as pending removal after a successful
switch and expected boarding in both worlds. The original target and its assigned
destination slots must remain unchanged, the target must differ from home, and
there must be no arrival or destination-invalidation evidence.

- Pause all new proposals immediately; retain the passenger's original identity,
  consumed attempt and destination. Clearing alone never resolves an outcome.
- Permit at most32 subsequent callbacks, measured from the first observation;
  repeated observations do not extend the deadline. No extra queries are needed.
- Resume only after both worlds report absence of SimPerson, all other pending
  cases resolve, and no permanent read-error block exists. Use the existing
  `passenger_removed` outcome, never an arrival or refunded attempt.
- Stop on deadline expiry, a restored or different home in a world that cleared
  it, reappearance after removal, mixed arrival/invalidation evidence, or changed
  target. The other replica may temporarily retain its original home.

`pilot_home_pending`, `pilot_home_timeout` and a per-passenger cleared-home mask
record the distinction. The startup log reports the fixed32-callback limit.
Home-bound journeys and clearing before confirmed boarding remain unsupported
and stop. The global callback cap still applies. ABI7, mutation implementation,
S1↔S3 leg eligibility, attempt cap and reserve option are unchanged.

## Validation and delivery

Normal and reserved consumer fixtures pass: actual later requests after confirmed
removal, the reserved fifth attempt, no premature release, exact deadline expiry,
late removal, restoration/reassignment, mixed evidence, multiple pending people,
permanent errors, excluded IDs and final-callback behavior. Host batch/failure
and both DLL compatibility suites also pass. Peak fixture usage remains40/64
queries. Build warnings are errors.

Prepared in ignored `analysis/native-bridge/build-v7e/` (commit4d6e70a); **not loaded**.
The separate0.7.3 diagnostic capture has now stopped; its DLL remains mapped
until game exit. Do not infer live validation from these fixtures.

SHA-256 evidence:

- Mutable DLL: `7f136927850b48987d51299aad4f9118398e7f7823331ff9e0dcec4eeb57d2a1`.
- Read-only DLL: `74b30e648528c5290f1b43743a3e735a09170d2703dcb62629d7e98414b05264`.
- `home-clear-final-tests.json`: `3f32e6a354c7755dc2576361de0bf6382f6523d6ef4e4c3b8c2cb8f4bc442871`.
- `analysis/native-boarding/home-invalid-removal.txt`:
  `06e5856c0fed780c2db1675425e083f8d7b3947f8e5454a7bf4a1b2703144ec4`.
- Diagnostic `identity-transition.jsonl`:
  `1769b6c3412e831b3c12ee70bb3bc28b4a6fa81ba8614aef3b337aae48b0f8c2`.

The external monitor ended at its600-second bound,17:18:36.890100Z:11,271 accepted
world samples and13 read rejections. Repeated copied reads are observational,
not atomic. The native diagnostic trial remains separately bounded at18,000
callbacks; its final outcome is in the continuation record below.


## Subsequent live diagnostic confirms the pending state

The diagnostic build stopped at7909 on120842: home75531→-1, destination slots
`[-1,78804,-1]`, target78804 unchanged, both-world boarding already confirmed.
This matches the new pending-state gate. A copied follow-up approximately51sec
later found no SimPerson in either world. The native removal time and whether it
fits within32 callbacks remain unknown;0.7.4 still needs a live trial. No natural
cause or original arrival is inferred from absence. The run recorded four
boardings and three original arrivals, with no fifth switch. All watchers ended.
See [complete diagnostic evidence and hashes](lifecycle-continuation.md#diagnostic-outcome--exact-cleared-home-state-captured).

## Live trial startup — 2026-10-08

Loaded0.7.4 from build-v7e into fresh Linux93030/start2190548 / Windows304 after
verifying the correct before_multi_passenger baseline and no previous bridge.
Stdout load17:40:52Z, ready17:41:37Z. DLL hash matches the tested artifact above;
loader tfx_start0. Log `tpf-bridge-304-22022778.log` confirms cap5/18000, reserve1
and home-clear grace32. User confirmed normal-speed unpause; live outcome pending.
World1 `0x33227050`, world2 `0x33227d10`, thread624; log watcher48734 active.
Metadata is adjacent live-run.json; do not reuse older worlds/process identities.
