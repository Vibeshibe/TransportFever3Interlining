#ifndef TFX_CANDIDATE_PROBE_H
#define TFX_CANDIDATE_PROBE_H
#include "include/tpf_bridge.h"
tfx_status tfx_register_candidate_probe(void (*write_line)(const char *));
#endif
