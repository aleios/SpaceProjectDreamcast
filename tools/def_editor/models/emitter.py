from PyQt6.QtCore import QAbstractTableModel, QModelIndex, Qt


class EmitterModel(QAbstractTableModel):
    COL_PROJECTILE, COL_SPAWNS, COL_DELAY, COL_START_ANGLE, COL_STEP_ANGLE, COL_SPEED, COL_LIFETIME, COL_OFFSETX, COL_OFFSETY = range(9)

    MAP = {
        COL_PROJECTILE: {'key': 'projectile', 'type': str},
        COL_SPAWNS: {'key': 'spawns_per_step', 'type': int},
        COL_DELAY: {'key': 'delay', 'type': int},
        COL_START_ANGLE: {'key': 'start_angle', 'type': float},
        COL_STEP_ANGLE: {'key': 'step_angle', 'type': float},
        COL_SPEED: {'key': 'speed', 'type': float},
        COL_LIFETIME: { 'key': 'lifetime', 'type': int },
    }

    def __init__(self, parent=None):
        super().__init__(parent)
        self.data_dict = {}

    def rowCount(self, parent=QModelIndex()):
        return 1

    def columnCount(self, parent=QModelIndex()):
        return 9

    def set_data(self, data):
        self.beginResetModel()
        self.data_dict = data
        if 'offset' not in self.data_dict or not isinstance(self.data_dict['offset'], list) or len(self.data_dict['offset']) < 2:
            self.data_dict['offset'] = [0.0, 0.0]
        self.endResetModel()

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None

        col = index.column()
        if col == self.COL_OFFSETX:
            return self.data_dict['offset'][0]
        if col == self.COL_OFFSETY:
            return self.data_dict['offset'][1]

        key = self.MAP[col]['key']
        return self.data_dict.get(key)

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if not index.isValid() or role != Qt.ItemDataRole.EditRole:
            return False

        col = index.column()
        if col == self.COL_OFFSETX:
            self.data_dict['offset'][0] = float(value)
        elif col == self.COL_OFFSETY:
            self.data_dict['offset'][1] = float(value)
        else:
            key = self.MAP[col]['key']
            val_type = self.MAP[col]['type']
            try:
                self.data_dict[key] = val_type(value)
            except (ValueError, TypeError):
                return False

        self.dataChanged.emit(index, index, [role])
        return True

    def flags(self, index):
        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags
        return Qt.ItemFlag.ItemIsEnabled | Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsEditable