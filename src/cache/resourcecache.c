#include "resourcecache.h"
#include <stdlib.h>
#include <string.h>
#include <sh4zam/shz_cdefs.h>

#define FNV_PRIME  0x01000193
#define FNV_OFFSET 0x811c9dc5
#define CACHE_INITIAL_CAPACITY 32

void resourcecache_init(resourcecache_t* cache) {
    cache->total_entries = 0;
    cache->capacity = CACHE_INITIAL_CAPACITY;
    cache->entries = calloc(cache->capacity, sizeof(resourcecache_entry_t));
}

void resourcecache_destroy(resourcecache_t* cache) {

    // Free the allocated keys.
    for(uint32_t i = 0; i < cache->total_entries; ++i) {
        free(cache->entries[i].key);
    }

    // Free the entries
    free(cache->entries);
    cache->capacity = 0;
    cache->total_entries = 0;
}

uint32_t resourcecache_get_hash(const char* key) {
    // FNV-1a implementation
    uint32_t hash = FNV_OFFSET;
    for(int i = 0; i < strlen(key); ++i) {
        hash ^= (uint8_t)key[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

uint32_t resourcecache_get_index(resourcecache_t* cache, uint32_t hash) {
    return hash & (cache->capacity - 1);
}

bool resourcecache_exists(resourcecache_t* cache, const char* key) {

    return false;
}

void* resourcecache_get(resourcecache_t* cache, const char* key) {
    if (cache->capacity == 0) return NULL;

    uint32_t hash = resourcecache_get_hash(key);
    uint32_t index = resourcecache_get_index(cache, hash);
    uint32_t start_index = index;

    while (cache->entries[index].key != NULL) {
        if (strcmp(cache->entries[index].key, key) == 0) {
            return cache->entries[index].data;
        }

        index = (index + 1) % cache->capacity;
        if (index == start_index)
            break; // Wrapped around
    }

    return NULL;
}

resourcecache_entry_t* resourcecache_set(resourcecache_t* cache, const char* key, void* value) {
    if(!key || !cache || !cache->entries) {
        return nullptr;
    }

    //TODO: Dedup
    uint32_t hash = resourcecache_get_hash(key);
    uint32_t idx = resourcecache_get_index(cache, hash);

    // Try find existing entry
    while(cache->entries[idx].key != NULL) {
        int cmp = strcmp(key, cache->entries[idx].key);
        if(cmp == 0) {
            cache->entries[idx].data = value;
            return &cache->entries[idx];
        }
        idx++;
        // Wrap around if necessary
        if(idx >= cache->capacity) {
            idx = 0;
        }
    }

    // No entry found, add it.
    cache->entries[idx].key = strdup(key);
    cache->entries[idx].data = value;
    return &cache->entries[idx];
}

resourcecache_entry_t* resourcecache_get_or_insert(resourcecache_t* cache, const char* key) {

    if(SHZ_UNLIKELY(cache->capacity) == 0) {
        resourcecache_init(cache);
    }

    uint32_t hash = resourcecache_get_hash(key);
    uint32_t idx = resourcecache_get_index(cache, hash);

    // Check for entries
    while(cache->entries[idx].key != nullptr) {

        // Compare key for a match.
        const int cmp = strcmp(key, cache->entries[idx].key);
        if(cmp == 0) {
            return &cache->entries[idx];
        }

        // Keep going otherwise.
        idx++;
        if(idx >= cache->capacity) {
            idx = 0;
        }
    }

    return resourcecache_set(cache, key, nullptr);
}

bool resourcecache_release(resourcecache_t* cache, void* res) {
    if(!cache || !res) {
        return false;
    }

    // TODO: Implement with ref count

    return true;
}