from PyQt6.QtCore import QAbstractTableModel, QModelIndex, Qt

class EmitterModel(QAbstractTableModel):
    COL_NAME, COL_PROJECTILE, COL_SPAWNS, COL_DELAY, COL_START_ANGLE, COL_STEP_ANGLE, COL_SPEED, COL_LIFETIME, COL_OFFSETX, COL_OFFSETY = range(10)
    COL_TARGET, COL_TARGET_TRACKING, COL_TRACKING_DELAY = range(10, 13)

    MAP = {
        COL_NAME: {'key': 'name', 'type': str, 'default': 'Unnamed', 'export_cond': lambda d: False },
        COL_PROJECTILE: {'key': 'projectile', 'type': str, 'default': ''},
        COL_SPAWNS: {'key': 'spawns_per_step', 'type': int, 'default': 1},
        COL_DELAY: {'key': 'delay', 'type': int, 'default': 500 },
        COL_START_ANGLE: {'key': 'start_angle', 'type': float, 'default': 0.0},
        COL_STEP_ANGLE: {'key': 'step_angle', 'type': float, 'default': 0.0},
        COL_SPEED: {'key': 'speed', 'type': float, 'default': 1.0},
        COL_LIFETIME: { 'key': 'lifetime', 'type': int, 'default': 300 },
        COL_OFFSETX: {
            'key': 'offset_x', 'type': float, 'default': 0.0,
            'export_cond': lambda d: False
        },
        COL_OFFSETY: {
            'key': 'offset_y', 'type': float, 'default': 0.0,
            'export_cond': lambda d: False
        },

        COL_TARGET: {'key': 'target', 'type': int, 'default': 0},
        COL_TARGET_TRACKING: {
            'key': 'target_tracking', 'type': int, 'default': 0,
            'export_cond': lambda d: d.get('target', 0) > 0
        },
        COL_TRACKING_DELAY: {
            'key': 'tracking_delay', 'type': int, 'default': 0,
            'export_cond': lambda d: d.get('target', 0) > 0
        }
    }

    def __init__(self, parent=None):
        super().__init__(parent)
        self.emitters = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.emitters)

    def columnCount(self, parent=QModelIndex()):
        return 13

    def set_emitters(self, emitters):
        self.beginResetModel()
        self.emitters = emitters
        
        for emitter in self.emitters:
            # Ensure all keys from MAP exist in data_dict with defaults if missing
            for col, info in self.MAP.items():
                key = info['key']
                if key not in emitter:
                    emitter[key] = info.get('default')

            if 'offset' not in emitter or not isinstance(emitter['offset'], list) or len(emitter['offset']) < 2:
                emitter['offset'] = [0.0, 0.0]
        self.endResetModel()

    def add_emitter(self, emitter):
        row = len(self.emitters)
        self.beginInsertRows(QModelIndex(), row, row)
        self.emitters.append(emitter)
        
        # Ensure default values
        for col, info in self.MAP.items():
            key = info['key']
            if key not in emitter:
                emitter[key] = info.get('default')
        if 'offset' not in emitter:
            emitter['offset'] = [0.0, 0.0]
            
        self.endInsertRows()
        return self.index(row, 0)

    def remove(self, row):
        if 0 <= row < len(self.emitters):
            self.beginRemoveRows(QModelIndex(), row, row)
            self.emitters.pop(row)
            self.endRemoveRows()

    def shift_up(self, row):
        if row > 0:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)
            self.emitters[row], self.emitters[row - 1] = self.emitters[row - 1], self.emitters[row]
            self.endMoveRows()
            return True
        return False

    def shift_down(self, row):
        if row < len(self.emitters) - 1:
            # Note: to move to after row+1, we must specify row+2 as destination
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2)
            self.emitters[row], self.emitters[row + 1] = self.emitters[row + 1], self.emitters[row]
            self.endMoveRows()
            return True
        return False

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None

        row = index.row()
        col = index.column()
        if not (0 <= row < len(self.emitters)):
            return None
            
        emitter = self.emitters[row]

        if col == self.COL_OFFSETX:
            return emitter.get('offset', [0.0, 0.0])[0]
        if col == self.COL_OFFSETY:
            return emitter.get('offset', [0.0, 0.0])[1]

        key = self.MAP[col]['key']
        default = self.MAP[col].get('default')
        return emitter.get(key, default)

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if not index.isValid() or role != Qt.ItemDataRole.EditRole:
            return False

        row = index.row()
        col = index.column()
        if not (0 <= row < len(self.emitters)):
            return False

        emitter = self.emitters[row]

        if col == self.COL_OFFSETX or col == self.COL_OFFSETY:
            if 'offset' not in emitter or not isinstance(emitter['offset'], list) or len(emitter['offset']) < 2:
                emitter['offset'] = [0.0, 0.0]
            
            if col == self.COL_OFFSETX:
                emitter['offset'][0] = float(value)
            else:
                emitter['offset'][1] = float(value)
        else:
            key = self.MAP[col]['key']
            val_type = self.MAP[col]['type']
            try:
                emitter[key] = val_type(value)
            except (ValueError, TypeError):
                return False

        self.dataChanged.emit(index, index, [role])
        return True

    def flags(self, index):
        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags
        if index.column() == self.COL_NAME:
            return Qt.ItemFlag.ItemIsEnabled
        return Qt.ItemFlag.ItemIsEnabled | Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsEditable

    def _should_export(self, col, d):
        data = self.MAP[col]
        pred = data.get('export_cond', None)
        return True if pred is None else bool(pred(d))

    def export_data(self, row) -> dict:
        if not (0 <= row < len(self.emitters)):
            return {}
            
        src = self.emitters[row]
        out = dict(src)
        
        # Ensure all mapped keys are present
        for col, data in self.MAP.items():
            key = data['key']
            if self._should_export(col, out):
                if key not in out:
                    out[key] = data.get('default')
            else:
                out.pop(key, None)

        return out
