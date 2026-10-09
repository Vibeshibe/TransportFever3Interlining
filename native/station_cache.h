#ifndef TFX_STATION_CACHE_H
#define TFX_STATION_CACHE_H
#include <stdint.h>
#include <string.h>

/* Discovery hints only. Fresh copied queries must authorize every proposal. */
#define TFX_STATION_BUCKETS 16u
#define TFX_STATION_PEOPLE 32u
typedef struct tfx_station_hint { int32_t person, line; } tfx_station_hint;
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
static inline void tfx_station_remember(tfx_station_cache *cache, int32_t person,
                                      int32_t station_group, int32_t line) {
    if (person < 0 || station_group < 0 || line < 0) return;
    int32_t target = -1;
    for (uint32_t b = 0; b < cache->count; ++b) {
        tfx_station_bucket *bucket = &cache->buckets[b];
        if (bucket->station_group == station_group) target = (int32_t)b;
        for (uint32_t i = 0; i < bucket->count; ++i) {
            if (bucket->people[i].person != person) continue;
            if (bucket->station_group == station_group) {
                bucket->people[i].line = line;
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
    bucket->people[slot] = (tfx_station_hint){person, line};
}
static inline int32_t tfx_station_next(tfx_station_cache *cache, int32_t station_group,
                                     int32_t arriving_line, const int32_t *checked,
                                     uint32_t checked_count) {
    for (uint32_t b = 0; b < cache->count; ++b) {
        tfx_station_bucket *bucket = &cache->buckets[b];
        if (bucket->station_group != station_group) continue;
        for (uint32_t n = 0; n < bucket->count; ++n) {
            const tfx_station_hint *hint = &bucket->people[bucket->read_cursor++ % bucket->count];
            if (hint->person < 0 || hint->line == arriving_line) continue;
            uint32_t duplicate = 0;
            for (uint32_t i = 0; i < checked_count; ++i) duplicate |= checked[i] == hint->person;
            if (!duplicate) return hint->person;
        }
        break;
    }
    return -1;
}
#endif
