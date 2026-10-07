#ifndef TFX_HOST_H
#define TFX_HOST_H
#include "include/tpf_bridge.h"
#include "queries.h"
/* Backend-only entry. Rejects overlapping dispatch; callbacks run on caller. */
tfx_status tfx_dispatch(uint32_t delta_seconds_bits);
tfx_status tfx_dispatch_with_reader(uint32_t delta_seconds_bits, tfx_reader *reader);
#endif
