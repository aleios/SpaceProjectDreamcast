#include "caches.h"
#include "resourcecache.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define DEFINE_CACHE(name, type, init_fn, destroy_fn)                           \
resourcecache_t g_##name;                                                       \
type* name##_get(const char* key) {                                             \
    resourcecache_entry_t* entry = resourcecache_get_or_insert(&g_##name, key); \
    if(!entry->data) {                                                          \
        entry->data = malloc(sizeof(type));                                     \
        if(!init_fn(entry->data, key)) {                                        \
            free(entry->data);                                                  \
            entry->data = nullptr;                                              \
            printf("[%s]: Failed to load (%s)\n", TOSTRING(name), key);         \
            arch_abort();                                                       \
        }                                                                       \
    }                                                                           \
    return (type*)entry->data;                                                  \
}                                                                               \
void name##_release(type* data) {                                               \
    resourcecache_release(&g_##name, data);                                     \
}

// Rendering
DEFINE_CACHE(texcache, texture_t, texture_init, texture_destroy);
DEFINE_CACHE(animcache, animation_t, animation_init, animation_destroy);
DEFINE_CACHE(fontcache, spritefont_t, spritefont_init, spritefont_destroy);

// Defs
DEFINE_CACHE(projdefcache, projectiledef_t, projectiledef_init, projectiledef_destroy);
DEFINE_CACHE(enemydefcache, enemydef_t, enemydef_init, enemydef_destroy);