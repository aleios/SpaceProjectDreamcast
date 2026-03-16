#include "resourcecache.h"
#include <stdlib.h>
#include <string.h>
#include <sh4zam/shz_cdefs.h>

#define FNV_PRIME  0x01000193
#define FNV_OFFSET 0x811c9dc5
#define CACHE_INITIAL_CAPACITY 32
#define CACHE_LOAD_FACTOR 0.7f

static uint32_t resourcecache_get_index(resourcecache_t* cache, uint32_t hash) {
    return hash & (cache->capacity - 1);
}

static uint32_t resourcecache_get_hash(const char* key) {
    // FNV-1a implementation
    uint32_t hash = FNV_OFFSET;
    for(int i = 0; i < strlen(key); ++i) {
        hash ^= (uint8_t)key[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

static resourcecache_entry_t* resourcecache_find_entry(resourcecache_t* cache, const char* key)
{
    if (!cache || !cache->entries || !key)
        return nullptr;

    const auto hash = resourcecache_get_hash(key);
    auto idx  = resourcecache_get_index(cache, hash);

    for (;;) {
        const auto entry = &cache->entries[idx];

        if (!entry->key || strcmp(entry->key, key) == 0)
            return entry;

        idx++;
        if (idx == cache->capacity)
            idx = 0;
    }
}

static void resourcecache_ensure(resourcecache_t* cache) {
    if ((float)cache->total_entries / (float)cache->capacity > CACHE_LOAD_FACTOR) {

        // Calc new capacity. Just doubling for now
        const auto old_capacity = cache->capacity;
        cache->capacity = cache->capacity * 2;
        const auto old_entries = cache->entries;

        // Setup new cache
        cache->entries = calloc(cache->capacity, sizeof(resourcecache_entry_t));
        cache->total_entries = 0;

        // Rehash and reset entry count
        for (int i = 0; i < old_capacity; ++i) {
            const auto old_entry = &cache->entries[i];

            if (!old_entry->key) {
                continue;
            }

            const auto entry = resourcecache_find_entry(cache, old_entry->key);

            entry->key = old_entry->key;
            entry->data = old_entry->data;

            cache->total_entries++;
        }
        free(old_entries);
    }
}

void resourcecache_init(resourcecache_t* cache) {
    cache->total_entries = 0;
    cache->capacity = CACHE_INITIAL_CAPACITY;
    cache->entries = calloc(cache->capacity, sizeof(resourcecache_entry_t));
}

void resourcecache_destroy(resourcecache_t* cache) {

    // Free the allocated keys.
    for(uint32_t i = 0; i < cache->capacity; ++i) {
        if (cache->entries[i].key) {
            free(cache->entries[i].key);
        }
    }

    // Free the entries
    free(cache->entries);
    cache->capacity = 0;
    cache->total_entries = 0;
}

bool resourcecache_exists(resourcecache_t* cache, const char* key) {
    const auto entry = resourcecache_find_entry(cache, key);
    return entry && entry->key != nullptr;
}

void* resourcecache_get(resourcecache_t* cache, const char* key) {
    if (!cache || !cache->entries || !cache->capacity)
        return nullptr;

    const auto entry = resourcecache_find_entry(cache, key);

    return (entry->key) ? entry->data : nullptr;
}

resourcecache_entry_t* resourcecache_set(resourcecache_t* cache, const char* key, void* value) {
    if(!cache || !cache->entries) {
        return nullptr;
    }

    resourcecache_ensure(cache);
    const auto entry = resourcecache_find_entry(cache, key);

    if (entry->key == nullptr) {
        entry->key = strdup(key);
        cache->total_entries++;
    }

    entry->data = value;
    return entry;
}

resourcecache_entry_t* resourcecache_get_or_insert(resourcecache_t* cache, const char* key) {

    if(cache->capacity == 0) {
        resourcecache_init(cache);
    }

    resourcecache_ensure(cache);
    const auto entry = resourcecache_find_entry(cache, key);

    if (entry->key == nullptr) {
        entry->key = strdup(key);
        entry->data = nullptr;
        cache->total_entries++;
    }
    return entry;
}

bool resourcecache_release(resourcecache_t* cache, void* res) {
    if(!cache || !res) {
        return false;
    }

    // TODO: Implement with ref count

    return true;
}