#ifndef TFX_PILOT_CACHE_H
#define TFX_PILOT_CACHE_H
#include <stdint.h>
#define TFX_PILOT_PEOPLE 32u
#define TFX_PILOT_VEHICLES 128u
#define TFX_PILOT_LINES 64u
#define TFX_READY_HINTS 16u
/* Bounded discovery IDs, not a route/eligibility cache. Return the
 * replaced/inserted slot, or -1 when the ID is already remembered/invalid. */
static inline int32_t tfx_pilot_remember_bounded(int32_t *ids, uint32_t *count,
                                               uint32_t *cursor, uint32_t limit, int32_t person) {
    if (person < 0 || !limit || *count > limit) return -1;
    for (uint32_t i = 0; i < *count; ++i) if (ids[i] == person) return -1;
    uint32_t slot = *count;
    for (uint32_t i = 0; i < *count; ++i) if (ids[i] < 0) { slot = i; break; }
    if (slot == *count) slot = *count < limit ? (*count)++ : (*cursor)++ % limit;
    ids[slot] = person;
    return (int32_t)slot;
}
static inline int32_t tfx_pilot_remember(int32_t ids[TFX_PILOT_PEOPLE], uint32_t *count,
                                       uint32_t *cursor, int32_t person) {
    return tfx_pilot_remember_bounded(ids, count, cursor, TFX_PILOT_PEOPLE, person);
}
#endif
