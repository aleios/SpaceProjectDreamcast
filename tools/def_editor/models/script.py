import glob
import os
from pathlib import Path
from PyQt6.QtCore import QAbstractTableModel, QModelIndex, Qt

class ScriptModel(QAbstractTableModel):
    COL_NAME = 0
    COL_SOURCE = 1

    def __init__(self):
        super().__init__()
        self.items = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.items)

    def columnCount(self, parent=QModelIndex()):
        return 2

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        script = self.items[index.row()]
        col = index.column()
        if col == self.COL_NAME: return script['name']
        if col == self.COL_SOURCE: return script['source']
        return None

    def load(self, assets_path):
        path = os.path.join(assets_path, "scripts", "*.lua")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                source = Path(file_path).read_text(encoding='utf-8')
                self.items.append({
                    'name': name,
                    'source': source
                })