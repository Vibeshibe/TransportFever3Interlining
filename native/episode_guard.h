#ifndef TFX_EPISODE_GUARD_H
#define TFX_EPISODE_GUARD_H
#include <stdlib.h>
#include <string.h>
#include "include/tpf_bridge.h"

/* Sparse radix storage follows the query layer's 24-bit entity range. Three
 * indexed lookups, no lifetime scan, eviction, or relocation of live entries.
 * Allocate 256 entries at a time; allocation failure must precede mutation. */
typedef struct tfx_episode {
    uint64_t arrival, previous_arrival;
    int32_t destination, line, vehicle, boarding_stop, alighting_stop;
    uint32_t used, has_previous, boarded;
} tfx_episode;
typedef struct tfx_episode_page { tfx_episode entries[256]; } tfx_episode_page;
typedef struct tfx_episode_branch { tfx_episode_page *pages[256]; } tfx_episode_branch;
typedef struct tfx_episode_map {
    tfx_episode_branch *branches[256];
    size_t allocated_bytes;
} tfx_episode_map;
static inline tfx_episode *tfx_episode_get(tfx_episode_map *map, int32_t person, int allocate) {
    if (person < 0 || (uint32_t)person >= (1u << 24)) return NULL;
    uint32_t a = (uint32_t)person >> 16, b = ((uint32_t)person >> 8) & 255u;
    if (!map->branches[a] && allocate) {
        map->branches[a] = calloc(1, sizeof(tfx_episode_branch));
        if (map->branches[a]) map->allocated_bytes += sizeof(tfx_episode_branch);
    }
    tfx_episode_branch *branch = map->branches[a];
    if (!branch) return NULL;
    if (!branch->pages[b] && allocate) {
        branch->pages[b] = calloc(1, sizeof(tfx_episode_page));
        if (branch->pages[b]) map->allocated_bytes += sizeof(tfx_episode_page);
    }
    return branch->pages[b] ? &branch->pages[b]->entries[(uint32_t)person & 255u] : NULL;
}
static inline int tfx_episode_allowed(const tfx_episode *entry, const tfx_waiting_leg *leg) {
    return !entry || !entry->used || (entry->boarded == 3u &&
        leg->arrival_time_bits != entry->arrival &&
        (!entry->has_previous || leg->arrival_time_bits != entry->previous_arrival));
}
static inline void tfx_episode_begin(tfx_episode *entry, const tfx_waiting_leg_change *change) {
    uint32_t used = entry->used;
    uint64_t previous = entry->arrival;
    *entry = (tfx_episode){.arrival = change->expected.arrival_time_bits,
        .previous_arrival = previous, .has_previous = used, .used = 1,
        .destination = change->expected.destination, .line = change->line,
        .vehicle = change->candidate_vehicle, .boarding_stop = change->boarding_stop,
        .alighting_stop = change->alighting_stop};
}
static inline void tfx_episode_observe(tfx_episode *entry, const tfx_person_journey *journey) {
    if (!entry || !entry->used || journey->world_id < 1 || journey->world_id > 2 ||
        journey->flags != TFX_JOURNEY_ONBOARD || journey->destination != entry->destination ||
        journey->line != entry->line || journey->vehicle != entry->vehicle ||
        journey->boarding_stop != entry->boarding_stop || journey->alighting_stop != entry->alighting_stop) return;
    entry->boarded |= 1u << (journey->world_id - 1);
}
static inline void tfx_episode_clear(tfx_episode_map *map) {
    for (unsigned a = 0; a < 256; ++a) if (map->branches[a]) {
        for (unsigned b = 0; b < 256; ++b) free(map->branches[a]->pages[b]);
        free(map->branches[a]);
    }
    memset(map, 0, sizeof(*map));
}
#endif
