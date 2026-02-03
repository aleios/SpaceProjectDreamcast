import os
import glob
from PyQt6.QtCore import QModelIndex, QAbstractListModel, Qt

class SpritesModel(QAbstractListModel):
    def __init__(self):
        super().__init__()
        self.fonts = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.fonts)

    def columnCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        return self.fonts[index.row()]

    def load(self, assets_path):
        path = os.path.join(assets_path, "sprites", "*.png")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                self.fonts.append(name)