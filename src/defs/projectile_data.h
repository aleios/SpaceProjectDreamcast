#pragma once

typedef enum ProjectileTrackingType {
    PROJECTILETRACKINGTYPE_NONE,            //< No tracking
    PROJECTILETRACKINGTYPE_SNAPSHOT,        //< Position acquired at spawn
    PROJECTILETRACKINGTYPE_ACQUIRE_ONCE,    //< Position acquired after targeting_delay
    PROJECTILETRACKINGTYPE_CONTINUOUS       //< Position continuously updated
} projectiletrackingtype_t;