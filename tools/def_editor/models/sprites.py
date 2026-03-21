import os
import glob
from PyQt6.QtCore import QModelIndex, QAbstractListModel, Qt

def make_texture_path(texture_name):
    from tools.def_editor.defsdb import assets_path
    return os.path.join(assets_path, "sprites", f"{texture_name}.png")

class SpritesModel(QAbstractListModel):
    def __init__(self):
        super().__init__()
        self.sprites = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.sprites)

    def columnCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        return self.sprites[index.row()]

    def load(self, assets_path):
        path = os.path.join(assets_path, "sprites", "*.png")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                self.sprites.append(name)

    def find_by_name(self, name):
        for x in self.sprites:
            if x['name'] == name:
                return x
        return None