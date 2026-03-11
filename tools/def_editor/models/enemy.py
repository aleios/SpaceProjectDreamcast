from PyQt6.QtCore import QAbstractTableModel, QModelIndex, Qt

from .datadef import DefModel

TOTAL_WEAPON_SLOTS = 5

class WeaponSlotModel(QAbstractTableModel):
    def __init__(self, parent_model, parent_row):
        super().__init__()
        self.current_row = parent_row
        self.parent_model = parent_model
        self.slots = parent_model._data_list[parent_row]['weapon_slots']

    def rowCount(self, parent=QModelIndex()):
        return 1

    def columnCount(self, parent=QModelIndex()):
        return len(self.slots)

    def set_row(self, row):
        self.current_row = row
        self.slots = self.parent_model._data_list[row]['weapon_slots']
        self.dataChanged.emit(self.index(0, 0), self.index(0, len(self.slots)-1))

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid():
            return None
        return self.slots[index.column()]

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if not index.isValid():
            return False
        if role in [Qt.ItemDataRole.EditRole, Qt.ItemDataRole.DisplayRole]:
            self.slots[index.column()] = value
            self.dataChanged.emit(index, index, [role])
            self.notify_changed()
            return True
        return False

    def notify_changed(self):
        idx = self.parent_model.index(self.current_row, EnemyModel.COL_MODIFIED)
        self.parent_model.setData(idx, True)

class EnemyModel(DefModel):
    (COL_NAME, COL_MODIFIED, COL_ANIM, COL_IDLE_KEY, COL_LEFT_KEY, COL_RIGHT_KEY,
     COL_HEALTH, COL_COLLISION_RADIUS, COL_SCORE, COL_SCRIPT, COL_WEAPONSLOTS) = range(11)
    MAP = {
        COL_NAME: {'key': 'name', 'type': str},
        COL_MODIFIED: {'key': 'modified', 'type': bool},
        COL_ANIM: {'key': 'animation', 'type': str},
        COL_IDLE_KEY: {'key': 'idle_key', 'type': str},
        COL_LEFT_KEY: {'key': 'left_key', 'type': str},
        COL_RIGHT_KEY: {'key': 'right_key', 'type': str},
        COL_HEALTH: { 'key': 'health', 'type': int, 'default': 1 },
        COL_COLLISION_RADIUS: { 'key': 'collision_radius', 'type': float },
        COL_SCORE: { 'key': 'score', 'type': int, 'default': 1 },
        COL_SCRIPT: { 'key': 'script', 'type': str },
        COL_WEAPONSLOTS: { 'key': 'weapon_slots', 'type': list, 'default': [""] * TOTAL_WEAPON_SLOTS }
    }
    def __init__(self, *args, **kwargs):
        super().__init__("enemy", self.MAP, *args, **kwargs)

    def set_animation(self, row, anim):
        self.setData(self.index(row, self.COL_ANIM), anim)

    def set_idle_key(self, row, key):
        self.setData(self.index(row, self.COL_IDLE_KEY), key)

    def set_left_key(self, row, key):
        self.setData(self.index(row, self.COL_LEFT_KEY), key)

    def set_right_key(self, row, key):
        self.setData(self.index(row, self.COL_RIGHT_KEY), key)

    def get_weaponslot_model(self, row):
        return WeaponSlotModel(self, row)