#pragma once
#include <stdint.h>

typedef struct ResourceCacheEntry {
    char* key;
    void* data;
} resourcecache_entry_t;

typedef struct ResourceCache {
    resourcecache_entry_t* entries;
    uint32_t total_entries;
    uint32_t capacity;
} resourcecache_t;


void resourcecache_init(resourcecache_t* cache);
void resourcecache_destroy(resourcecache_t* cache);

bool resourcecache_exists(resourcecache_t* cache, const char* key);

void* resourcecache_get(resourcecache_t* cache, const char* key);

resourcecache_entry_t* resourcecache_set(resourcecache_t* cache, const char* key, void* value);
resourcecache_entry_t* resourcecache_get_or_insert(resourcecache_t* cache, const char* key);

bool resourcecache_release(resourcecache_t* cache, void* res);