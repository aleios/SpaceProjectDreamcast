import json
import os

from PyQt6.QtCore import QAbstractTableModel, Qt

from tools.def_editor.models import WeaponSetModel


class PlayerModel(QAbstractTableModel):
    COL_ANIMATION, COL_IDLE_CLIP, COL_LEFT_CLIP, COL_RIGHT_CLIP, COL_SPEED, COL_WEAPONS = range(6)
    _data = {
        'animation': '',
        'idle_clip': '',
        'left_clip': '',
        'right_clip': '',
        'speed': 0.2,
        'weapons': []
    }

    def __init__(self, parent=None):
        super().__init__(parent)

    def rowCount(self, parent=None):
        return 1

    def columnCount(self, parent=None):
        return 6

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        col = index.column()
        if col == self.COL_ANIMATION: return self._data['animation']
        if col == self.COL_IDLE_CLIP: return self._data['idle_clip']
        if col == self.COL_LEFT_CLIP: return self._data['left_clip']
        if col == self.COL_RIGHT_CLIP: return self._data['right_clip']
        if col == self.COL_SPEED: return self._data['speed']
        if col == self.COL_WEAPONS: return self._data['weapons']
        return None

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if not index.isValid():
            return None

        col = index.column()
        if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            if col == self.COL_ANIMATION: self._data['animation'] = value
            if col == self.COL_IDLE_CLIP: self._data['idle_clip'] = value
            if col == self.COL_LEFT_CLIP: self._data['left_clip'] = value
            if col == self.COL_RIGHT_CLIP: self._data['right_clip'] = value
            if col == self.COL_SPEED: self._data['speed'] = float(value)
            self.dataChanged.emit(index, index, [role, Qt.ItemDataRole.DisplayRole])

            return True

        return False

    def load(self, assets_path):
        try:
            with open(os.path.join(assets_path, 'player.json'), 'r') as f:
                self.beginResetModel()
                self._data = json.load(f)
                self.endResetModel()
        except Exception as e:
            with open(os.path.join(assets_path, 'player.json'), 'w') as f:
                json.dump(self._data, f, indent=2, separators=(',', ': '))

    def save(self, assets_path):
        with open(os.path.join(assets_path, 'player.json'), 'w') as f:
            json.dump(self._data, f, indent=2, separators=(',', ': '))

    def make_weapons_model(self):
        return WeaponSetModel(self._data['weapons'])

    def set_animation(self, anim):
        self.setData(self.index(0, self.COL_ANIMATION), anim)

    def set_idle_clip(self, key):
        self.setData(self.index(0, self.COL_IDLE_CLIP), key)

    def set_left_clip(self, key):
        self.setData(self.index(0, self.COL_LEFT_CLIP), key)

    def set_right_clip(self, key):
        self.setData(self.index(0, self.COL_RIGHT_CLIP), key)