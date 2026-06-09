from .datadef import DefModel


class CollectablesModel(DefModel):
    (COL_NAME, COL_MODIFIED, COL_ANIM, COL_ANIM_KEY, COL_LIFETIME,
     COL_PICKUP_SOUND, COL_COLLIDER_RADIUS, COL_SPEED, COL_EFFECT_HEALTH, COL_EFFECT_LIVES, COL_EFFECT_WEAPON, COL_EFFECT_SCORE, COL_EFFECT_SCRIPT) = range(13)
    MAP = {
        COL_NAME: {'key': 'name', 'type': str},
        COL_MODIFIED: {'key': 'modified', 'type': bool},
        COL_ANIM: {'key': 'animation', 'type': str},
        COL_ANIM_KEY: {'key': 'animation_key', 'type': str},
        COL_LIFETIME: { 'key': 'lifetime', 'type': int, 'default': 5000 },
        COL_PICKUP_SOUND: { 'key': 'pickup_sound', 'type': str, 'default': '' },
        COL_COLLIDER_RADIUS: {'key': 'collider_radius', 'type': float, 'default': 1.0 },
        COL_SPEED: { 'key': 'speed', 'type': float, 'default': 0.1 },
        COL_EFFECT_HEALTH: {'key': 'health', 'type': int, 'default': 0 },
        COL_EFFECT_LIVES: {'key': 'lives', 'type': int, 'default': 0 },
        COL_EFFECT_WEAPON: {'key': 'weapon', 'type': int, 'default': 0 },
        COL_EFFECT_SCORE: {'key': 'score', 'type': int, 'default': 0 },
        COL_EFFECT_SCRIPT: {'key': 'script', 'type': str, 'default': ''},
    }

    def __init__(self, *args, **kwargs):
        super().__init__("collectable", self.MAP, *args, **kwargs)

    def get_script(self, row):
        if row < 0 or row >= len(self._data_list):
            return ''
        return self._data_list[row]['script']

    def set_script(self, row, script):
        if row < 0 or row >= len(self._data_list):
            return
        self.setData(self.index(row, self.COL_EFFECT_SCRIPT), script)
