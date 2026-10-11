#ifndef TFX_STATION_CACHE_H
#define TFX_STATION_CACHE_H
#include <stdint.h>
#include <string.h>

/* Discovery hints only. Fresh copied queries must authorize every proposal. */
#define TFX_STATION_BUCKETS 64u
#define TFX_STATION_PEOPLE 128u
typedef struct tfx_station_hint { int32_t person, line, boarding_stop, alighting_stop, alighting_group; } tfx_station_hint;
/* Separate cursors keep preferred legs from advancing general exploration.
 * Each arriving vehicle owns one search; these fields only rank fresh reads. */
typedef struct tfx_station_search {
    uint32_t cursor, preferred_cursor, selections;
    int32_t line, boarding_stop, alighting_stop, alighting_group;
    uint32_t reachable_count;
    int32_t reachable[4];
} tfx_station_search;
typedef struct tfx_station_bucket {
    int32_t station_group;
    uint32_t count, read_cursor, replace_cursor;
    tfx_station_hint people[TFX_STATION_PEOPLE];
} tfx_station_bucket;
typedef struct tfx_station_cache {
    uint32_t count, replace_cursor;
    tfx_station_bucket buckets[TFX_STATION_BUCKETS];
} tfx_station_cache;

static inline void tfx_station_forget(tfx_station_cache *cache, int32_t person) {
    for (uint32_t b = 0; b < cache->count; ++b)
        for (uint32_t i = 0; i < cache->buckets[b].count; ++i)
            if (cache->buckets[b].people[i].person == person)
                cache->buckets[b].people[i].person = -1;
}
static inline uint32_t tfx_station_bucket_size(const tfx_station_bucket *bucket) {
    uint32_t count = 0;
    for (uint32_t i = 0; i < bucket->count; ++i) count += bucket->people[i].person >= 0;
    return count;
}
static inline void tfx_station_remember_leg(tfx_station_cache *cache, int32_t person,
        int32_t station_group, int32_t line, int32_t boarding_stop, int32_t alighting_stop) {
    if (person < 0 || station_group < 0 || line < 0) return;
    int32_t target = -1;
    for (uint32_t b = 0; b < cache->count; ++b) {
        tfx_station_bucket *bucket = &cache->buckets[b];
        if (bucket->station_group == station_group) target = (int32_t)b;
        for (uint32_t i = 0; i < bucket->count; ++i) {
            if (bucket->people[i].person != person) continue;
            if (bucket->station_group == station_group) {
                if (bucket->people[i].line != line || bucket->people[i].boarding_stop != boarding_stop ||
                    bucket->people[i].alighting_stop != alighting_stop) bucket->people[i].alighting_group = -1;
                bucket->people[i].line = line;
                bucket->people[i].boarding_stop = boarding_stop;
                bucket->people[i].alighting_stop = alighting_stop;
                return; /* Refresh without resetting round-robin position. */
            }
            bucket->people[i].person = -1;
        }
    }
    if (target < 0) {
        for (uint32_t b = 0; b < cache->count; ++b)
            if (!tfx_station_bucket_size(&cache->buckets[b])) { target = (int32_t)b; break; }
        if (target < 0) target = (int32_t)(cache->count < TFX_STATION_BUCKETS ?
            cache->count++ : cache->replace_cursor++ % TFX_STATION_BUCKETS);
        memset(&cache->buckets[target], 0, sizeof(cache->buckets[target]));
        cache->buckets[target].station_group = station_group;
    }
    tfx_station_bucket *bucket = &cache->buckets[target];
    uint32_t slot = bucket->count;
    for (uint32_t i = 0; i < bucket->count; ++i)
        if (bucket->people[i].person < 0) { slot = i; break; }
    if (slot == bucket->count) slot = bucket->count < TFX_STATION_PEOPLE ?
        bucket->count++ : bucket->replace_cursor++ % TFX_STATION_PEOPLE;
    bucket->people[slot] = (tfx_station_hint){person, line, boarding_stop, alighting_stop, -1};
}
static inline void tfx_station_destination(tfx_station_cache *cache, int32_t person,
        int32_t station_group, int32_t alighting_group) {
    for (uint32_t b = 0; b < cache->count; ++b) {
        tfx_station_bucket *bucket = &cache->buckets[b];
        if (bucket->station_group != station_group) continue;
        for (uint32_t i = 0; i < bucket->count; ++i)
            if (bucket->people[i].person == person) { bucket->people[i].alighting_group = alighting_group; return; }
    }
}
static inline void tfx_station_remember(tfx_station_cache *cache, int32_t person,
                                      int32_t station_group, int32_t line) {
    tfx_station_remember_leg(cache, person, station_group, line, -1, -1);
}
static inline int32_t tfx_station_pick(tfx_station_cache *cache, int32_t station_group,
        int32_t arriving_line, uint32_t *cursor, const tfx_station_search *preferred,
        const int32_t *checked, uint32_t checked_count) {
    for (uint32_t b = 0; b < cache->count; ++b) {
        tfx_station_bucket *bucket = &cache->buckets[b];
        if (bucket->station_group != station_group) continue;
        /* Try another source line first within the preferred endpoint. A second
         * pass serves the previous source if no different source remains. */
        unsigned passes = preferred && (preferred->reachable_count || preferred->alighting_group >= 0) ? 2u : 1u;
        for (unsigned pass = 0; pass < passes; ++pass) for (uint32_t n = 0; n < bucket->count; ++n) {
            uint32_t *position = cursor ? cursor : &bucket->read_cursor;
            const tfx_station_hint *hint = &bucket->people[(*position)++ % bucket->count];
            if (hint->person < 0 || hint->line == arriving_line) continue;
            if (preferred) {
                if (preferred->reachable_count) {
                    unsigned reachable = 0;
                    for (unsigned k = 0; k < preferred->reachable_count && k < 4; ++k)
                        reachable |= hint->alighting_group == preferred->reachable[k];
                    if (!reachable || (!pass && hint->line == preferred->line)) continue;
                } else if (preferred->alighting_group >= 0) {
                    if (hint->alighting_group != preferred->alighting_group ||
                        (!pass && hint->line == preferred->line)) continue;
                } else if (hint->line != preferred->line || hint->boarding_stop != preferred->boarding_stop ||
                           hint->alighting_stop != preferred->alighting_stop) continue;
            }
            uint32_t duplicate = 0;
            for (uint32_t i = 0; i < checked_count; ++i) duplicate |= checked[i] == hint->person;
            if (!duplicate) return hint->person;
        }
        break;
    }
    return -1;
}
static inline int32_t tfx_station_next(tfx_station_cache *cache, int32_t station_group,
        int32_t arriving_line, const int32_t *checked, uint32_t checked_count) {
    return tfx_station_pick(cache, station_group, arriving_line, NULL, NULL, checked, checked_count);
}
#endif
