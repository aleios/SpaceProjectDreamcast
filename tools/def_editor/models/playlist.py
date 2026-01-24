from PyQt6.QtCore import QModelIndex, QAbstractTableModel, Qt
from tools.def_editor import defsdb


class PlaylistModel(QAbstractTableModel):
    mimeType = "application/x-playlist-item"

    def __init__(self, data_obj: defsdb.GameSettingsData, parent=None):
        super().__init__(parent)
        self.data_obj = data_obj

    def rowCount(self, parent=QModelIndex()):
        return len(self.data_obj.playlist)

    def columnCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None

        return self.data_obj.playlist[index.row()]

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            if self.data_obj.playlist[index.row()] != value:
                self.data_obj.playlist[index.row()] = value
                self.data_obj.modified = True
                self.dataChanged.emit(index, index, [role, Qt.ItemDataRole.DisplayRole])
            return True
        return False

    def supportedDropActions(self):
        return Qt.DropAction.MoveAction

    def flags(self, index):
        return super().flags(index) | Qt.ItemFlag.ItemIsDragEnabled | Qt.ItemFlag.ItemIsDropEnabled

    def add(self, level_name):
        self.beginInsertRows(QModelIndex(), len(self.data_obj.playlist), len(self.data_obj.playlist))
        self.data_obj.playlist.append(level_name)
        self.data_obj.modified = True
        self.endInsertRows()

    def remove(self, row):
        if 0 <= row < len(self.data_obj.playlist):
            self.beginRemoveRows(QModelIndex(), row, row)
            self.data_obj.playlist.pop(row)
            self.data_obj.modified = True
            self.endRemoveRows()

    def shift_up(self, row):
        if row > 0:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)
            self.data_obj.playlist[row], self.data_obj.playlist[row - 1] = self.data_obj.playlist[row - 1], self.data_obj.playlist[row]
            self.data_obj.modified = True
            self.endMoveRows()

    def shift_down(self, row):
        if row < len(self.data_obj.playlist) - 1:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2)
            self.data_obj.playlist[row], self.data_obj.playlist[row + 1] = self.data_obj.playlist[row + 1], self.data_obj.playlist[row]
            self.data_obj.modified = True
            self.endMoveRows()

    def mimeTypes(self):
        return [self.mimeType]

    def mimeData(self, indexes):
        data = QMimeData()
        encoded = str(indexes[0].row()).encode()
        data.setData(self.mimeType, encoded)
        return data

    def dropMimeData(self, data, action, row, column, parent):
        if action == Qt.DropAction.IgnoreAction:
            return True
        if not data.hasFormat(self.mimeType):
            return False

        src_row = int(data.data(self.mimeType).data().decode())
        dest_row = row if row != -1 else parent.row()

        if dest_row == -1:
            dest_row = self.rowCount()

        if src_row == dest_row or src_row == dest_row - 1:
            return False

        self.beginMoveRows(QModelIndex(), src_row, src_row, QModelIndex(), dest_row)

        item = self.data_obj.playlist.pop(src_row)
        insert_at = dest_row if dest_row <= src_row else dest_row - 1
        self.data_obj.playlist.insert(insert_at, item)
        self.data_obj.modified = True

        self.endMoveRows()
        return True