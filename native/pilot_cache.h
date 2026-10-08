#ifndef TFX_PILOT_CACHE_H
#define TFX_PILOT_CACHE_H
#include <stdint.h>
#define TFX_PILOT_PEOPLE 32u
/* Save-specific research discovery, not a route/eligibility cache. Return the
 * replaced/inserted slot, or -1 when the ID is already remembered/invalid. */
static inline int32_t tfx_pilot_remember(int32_t ids[TFX_PILOT_PEOPLE], uint32_t *count,
                                       uint32_t *cursor, int32_t person) {
    if (person < 0 || *count > TFX_PILOT_PEOPLE) return -1;
    for (uint32_t i = 0; i < *count; ++i) if (ids[i] == person) return -1;
    uint32_t slot = *count < TFX_PILOT_PEOPLE ? (*count)++ : (*cursor)++ % TFX_PILOT_PEOPLE;
    ids[slot] = person;
    return (int32_t)slot;
}
#endif
