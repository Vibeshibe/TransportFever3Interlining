#ifndef TFX_MUTATION_H
#define TFX_MUTATION_H
#include "host.h"
/* Private bindings. Not a general memory-write API. Fifth add argument is explicit. */
typedef struct tfx_engine_changes {
    void (*begin)(void *world);
    void (*remove)(void *world, int32_t person, const void *type, uint8_t notify);
    void (*add_waiting)(void *world, int32_t person, const void *value, uint8_t notify, uint8_t local_storage);
    void (*end)(void *world);
    void (*log)(const char *);
} tfx_engine_changes;
tfx_status tfx_apply_waiting_change(tfx_reader *, const tfx_waiting_leg_change *,
    const unsigned char expected_raw[32], uint32_t *attempted, void *operations);
#endif
