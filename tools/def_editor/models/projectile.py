from .datadef import DefModel

class ProjectileModel(DefModel):
    COL_NAME, COL_MODIFIED, COL_TEXTURE, COL_ANIM, COL_ANIM_KEY, COL_COLLIDER_RADIUS, COL_DAMAGE, COL_ROTATES = range(8)
    MAP = {
        COL_NAME: {'key': 'name', 'type': str},
        COL_MODIFIED: {'key': 'modified', 'type': bool},
        COL_TEXTURE: {'key': 'texture', 'type': str},
        COL_ANIM: {'key': 'animation', 'type': str},
        COL_ANIM_KEY: {'key': 'animation_key', 'type': str},
        COL_DAMAGE: { 'key': 'damage', 'type': int },
        COL_COLLIDER_RADIUS: { 'key': 'collider_radius', 'type': float, 'default': 1.0 },
        COL_ROTATES: { 'key': 'rotates', 'type': bool }
    }

    def __init__(self, *args, **kwargs):
        super().__init__("projectile", self.MAP, *args, **kwargs)

    def set_animation(self, row, anim):
        self.setData(self.index(row, self.COL_ANIM), anim)

    def set_animation_clip(self, row, clip):
        self.setData(self.index(row, self.COL_ANIM_KEY), clip)