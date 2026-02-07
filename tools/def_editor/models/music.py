import os
import glob
from PyQt6.QtCore import QModelIndex, QAbstractListModel, Qt

class MusicModel(QAbstractListModel):
    COL_NAME = 0

    def __init__(self):
        super().__init__()
        self.music = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.music)

    def columnCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        return self.music[index.row()]

    def load(self, assets_path):
        path = os.path.join(assets_path, "music", "*.adx")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                self.music.append(name)