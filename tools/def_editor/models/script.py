import copy
import glob
import os
from pathlib import Path
from PyQt6.QtCore import QAbstractTableModel, QModelIndex, Qt

META_KEYS = [
    'name',
    'modified'
]

class ScriptModel(QAbstractTableModel):
    COL_NAME, COL_MODIFIED, COL_SOURCE = range(3)
    _COLUMN_MAP = {
        COL_NAME: {'key': 'name', 'type': str},
        COL_MODIFIED: {'key': 'modified', 'type': bool},
        COL_SOURCE: {'key': 'source', 'type': str},
    }

    def __init__(self):
        super().__init__()
        self.items = []
        self._pending_deletions = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.items)

    def columnCount(self, parent=QModelIndex()):
        return len(self._COLUMN_MAP)

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        script = self.items[index.row()]
        col = index.column()
        if col == self.COL_NAME: return script['name']
        if col == self.COL_MODIFIED: return script['modified']
        if col == self.COL_SOURCE: return script['source']
        return None

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            row = index.row()
            col = index.column()
            col_info = self._COLUMN_MAP.get(col)

            if col_info:
                key = col_info['key']
                # Mark as modified if value changed.
                if self.items[row].get(key) != value:
                    self.items[row][key] = value

                    # Set to modified, unless modifying the modified field...
                    if key != 'modified':
                        self.items[row]['modified'] = True

                        mod_index = self.index(row, self.COL_MODIFIED)
                        self.dataChanged.emit(mod_index, mod_index, [Qt.ItemDataRole.DisplayRole])

                    self.dataChanged.emit(index, index, [role])
                    return True
        return False

    def load(self, assets_path):
        path = os.path.join(assets_path, "scripts", "*.lua")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                source = Path(file_path).read_text(encoding='utf-8')
                self.items.append({
                    "name": name,
                    "source": source,
                    "modified": False
                })

    def save(self, assets_path):
        base_path = os.path.join(assets_path, "scripts")
        os.makedirs(base_path, exist_ok=True)

        # Delete pending files
        while self._pending_deletions:
            p = self._pending_deletions.pop()
            if os.path.exists(p): os.remove(p)

        # Save script data to file.
        for row, item in enumerate(self.items):
            if item.get('modified'):
                path = os.path.join(base_path, f"{item['name']}.lua")
                save_data = copy.deepcopy(item)

                # Pop out the meta keys
                for k in META_KEYS: save_data.pop(k, None)

                with open(path, "w") as f:
                    f.write(save_data['source'])

                # Mark as unmodified and notify.
                item['modified'] = False
                if self.COL_MODIFIED != -1:
                    m_idx = self.index(row, self.COL_MODIFIED)
                    self.dataChanged.emit(m_idx, m_idx, [Qt.ItemDataRole.DisplayRole])

    def add(self, name):
        self.beginInsertRows(QModelIndex(), len(self.items), len(self.items))
        src = \
"""function init()
end

return {
    handlers = {
        init = init
    }
}"""
        self.items.append({
            "name": name,
            "source": src,
            "modified": True
        })
        from tools.def_editor import defsdb
        defsdb.scripts.dataChanged.emit(QModelIndex(), QModelIndex())
        self.endInsertRows()

    def exists(self, key):
        from tools.def_editor import defsdb
        return bool([x for x in self.items if x['name'] == key]) or os.path.isfile(f'{defsdb.assets_path}/scripts/{key}.lua')