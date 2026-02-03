from .datadef import DefModel


class CollectablesModel(DefModel):
    COL_NAME, COL_MODIFIED, COL_ANIM, COL_ANIM_KEY, COL_EFFECT_HEALTH, COL_EFFECT_LIVES, COL_EFFECT_WEAPON = range(7)
    MAP = {
        COL_NAME: {'key': 'name', 'type': str},
        COL_MODIFIED: {'key': 'modified', 'type': bool},
        COL_ANIM: {'key': 'animation', 'type': str},
        COL_ANIM_KEY: {'key': 'animation_key', 'type': str},
        COL_EFFECT_HEALTH: {'key': 'health', 'type': int},
        COL_EFFECT_LIVES: {'key': 'lives', 'type': int},
        COL_EFFECT_WEAPON: {'key': 'weapon', 'type': int}
    }

    def __init__(self, *args, **kwargs):
        super().__init__("collectable", self.MAP, *args, **kwargs)