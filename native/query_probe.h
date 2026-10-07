#ifndef TFX_QUERY_PROBE_H
#define TFX_QUERY_PROBE_H
#include "include/tpf_bridge.h"
tfx_status tfx_register_query_probe(void (*write_line)(const char *));
#endif
