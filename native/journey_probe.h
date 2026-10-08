#ifndef TFX_JOURNEY_PROBE_H
#define TFX_JOURNEY_PROBE_H
#include "include/tpf_bridge.h"
/* Explicit research target from a saved, previously observed journey. */
tfx_status tfx_register_journey_probe(void (*write_line)(const char *), int32_t person, int32_t destination);
int tfx_journey_probe_complete(void);
#endif
