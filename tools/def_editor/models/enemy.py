from .datadef import DefModel

class EnemyModel(DefModel):
    COL_NAME, COL_MODIFIED, COL_ANIM, COL_IDLE_KEY, COL_LEFT_KEY, COL_RIGHT_KEY, COL_HEALTH, COL_COLLISION_RADIUS, COL_EVENTS = range(9)
    MAP = {
        COL_NAME: {'key': 'name', 'type': str},
        COL_MODIFIED: {'key': 'modified', 'type': bool},
        COL_ANIM: {'key': 'animation', 'type': str},
        COL_IDLE_KEY: {'key': 'idle_key', 'type': str},
        COL_LEFT_KEY: {'key': 'left_key', 'type': str},
        COL_RIGHT_KEY: {'key': 'right_key', 'type': str},
        COL_HEALTH: { 'key': 'health', 'type': int },
        COL_COLLISION_RADIUS: { 'key': 'collision_radius', 'type': float },
        COL_EVENTS: { 'key': 'events', 'type': list }
    }
    def __init__(self, *args, **kwargs):
        super().__init__("enemy", self.MAP, *args, **kwargs)

    def add_event_cmd(self, index, cmd):
        pass

    def set_animation(self, row, anim):
        self.setData(self.index(row, self.COL_ANIM), anim)

    def set_idle_key(self, row, key):
        self.setData(self.index(row, self.COL_IDLE_KEY), key)

    def set_left_key(self, row, key):
        self.setData(self.index(row, self.COL_LEFT_KEY), key)

    def set_right_key(self, row, key):
        self.setData(self.index(row, self.COL_RIGHT_KEY), key)