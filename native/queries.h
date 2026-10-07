#ifndef TFX_QUERIES_H
#define TFX_QUERIES_H
#include <stddef.h>
#include "include/tpf_bridge.h"

/* Private, invocation-local data. This is NOT the mod ABI. Reads return exact
 * copies or fail. No game functions are called; no pointers are returned to mods. */
typedef int (*tfx_copy_memory)(void *user, uint64_t address, void *out, size_t size);
typedef struct tfx_reader {
    uint64_t world, image, terminal_system, station_system;
    tfx_copy_memory copy;
    void *user;
    uint32_t bytes_left, reads_left, queries_left;
    int types_ready;
    int32_t types[7];
} tfx_reader;

void tfx_reader_init(tfx_reader *, uint64_t world, uint64_t image, tfx_copy_memory, void *user);
tfx_status tfx_query_waiting(tfx_reader *, int32_t person, tfx_waiting_leg *out);
tfx_status tfx_query_stop(tfx_reader *, int32_t line, int32_t stop, tfx_line_stop *out);
tfx_status tfx_query_scan(tfx_reader *, int32_t first_entity, tfx_waiting_page *out);
tfx_status tfx_query_stop_vehicles(tfx_reader *, int32_t line, int32_t stop, tfx_stop_vehicles *out);
tfx_status tfx_query_vehicle(tfx_reader *, int32_t vehicle, tfx_vehicle *out);
tfx_status tfx_query_platforms(tfx_reader *, int32_t person, tfx_person_platforms *out);
tfx_status tfx_query_terminal(tfx_reader *, int32_t station, int32_t terminal, tfx_terminal *out);
tfx_status tfx_query_presence(tfx_reader *, int32_t entity, tfx_entity_presence *out);
#endif
