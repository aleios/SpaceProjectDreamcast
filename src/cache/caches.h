#pragma once
#include "../renderer/texture.h"
#include "../components/animation.h"
#include "../defs/projectile_def.h"
#include "../enemy/enemy_def.h"
#include "../renderer/sprite_font.h"
#include "../defs/collectable_def.h"
#include "../scripting/script.h"

// Rendering
texture_t* texcache_get(const char* key);
void texcache_release(texture_t* tex);

animation_t* animcache_get(const char* key);
void animcache_release(animation_t* anim);

spritefont_t* fontcache_get(const char* key);
void fontcache_release(spritefont_t* data);

// Defs
projectiledef_t* projdefcache_get(const char* key);
void projdefcache_release(projectiledef_t* data);

enemydef_t* enemydefcache_get(const char* key);
void enemydefcache_release(enemydef_t* data);

weaponsetdef_t* weaponsetcache_get(const char* key);
void weaponsetcache_release(weaponsetdef_t* data);

collectabledef_t* collectabledefcache_get(const char* key);
void collectabledefcache_release(collectabledef_t* data);

// Scripting
script_t* scriptcache_get(const char* key);
void scriptcache_release(script_t* data);