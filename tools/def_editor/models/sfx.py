import os
import glob
from PyQt6.QtCore import QModelIndex, QAbstractListModel, Qt

class SfxModel(QAbstractListModel):
    COL_NAME = 0

    def __init__(self):
        super().__init__()
        self.sfx = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.sfx)

    def columnCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        return self.sfx[index.row()]

    def load(self, assets_path):
        path = os.path.join(assets_path, "sfx", "*.wav")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                self.sfx.append(name)