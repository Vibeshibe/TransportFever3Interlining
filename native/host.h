#ifndef TFX_HOST_H
#define TFX_HOST_H
#include "include/tpf_bridge.h"
#include "queries.h"
/* Backend-only entry. Rejects overlapping dispatch; callbacks run on caller. */
tfx_status tfx_dispatch(uint32_t delta_seconds_bits);
tfx_status tfx_dispatch_with_reader(uint32_t delta_seconds_bits, tfx_reader *reader);
typedef tfx_status (*tfx_apply_change)(tfx_reader *, const tfx_waiting_leg_change *,
    const unsigned char expected_raw[32], uint32_t *engine_attempted, void *user);
/* Build-bounded distinct people; one attempt per dispatch, failure latches shut.
 * Read-only dispatchers provide no writer. */
tfx_status tfx_dispatch_with_writer(uint32_t, tfx_reader *, tfx_apply_change, void *);
#endif
