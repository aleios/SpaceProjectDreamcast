from PyQt6.QtCore import QAbstractTableModel, Qt, QModelIndex
import os
import glob
import json
import copy

META_KEYS = [
    'name',
    'modified'
]

# Base model that represents a file backed def
# Pass in json values to 'column_map', but don't pass in the meta keys.
class DefModel(QAbstractTableModel):
    def __init__(self, folder, column_map, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.folder = folder
        self._COLUMN_MAP = column_map
        self._data_list = []
        self._pending_deletions = []

        self.COL_MODIFIED = next((k for k, v in column_map.items() if v['key'] == 'modified'), -1)

    def rowCount(self, parent=QModelIndex()):
        return len(self._data_list)

    def columnCount(self, parent=None):
        return len(self._COLUMN_MAP)
    
    def data(self, index, role):
        if not index.isValid() or index.row() >= len(self._data_list):
            return None

        col = index.column()
        item = self._data_list[index.row()]

        if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            col_info = self._COLUMN_MAP.get(col)
            if col_info:
                val = item.get(col_info['key'], "")
                if col_info.get('type') is list:
                    return val
                return str(val)
        return None
    
    def setData(self, index, value, role = Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            row = index.row()
            col = index.column()
            col_info = self._COLUMN_MAP.get(col)
            
            if col_info:
                key = col_info['key']
                # Mark as modified if value changed.
                if self._data_list[row].get(key) != value:
                    self._data_list[row][key] = value
                    
                    # Set to modified, unless modifying the modified field...
                    if key != 'modified':
                        self._data_list[row]['modified'] = True
                        
                        mod_index = self.index(row, self.COL_MODIFIED)
                        self.dataChanged.emit(mod_index, mod_index, [Qt.ItemDataRole.DisplayRole])

                    self.dataChanged.emit(index, index, [role])
                    return True
        return False

    def flags(self, index):
        if not index.isValid():
            return None
        flags = super().flags(index)
        if not self._COLUMN_MAP[index.column()]['key'] in META_KEYS:
            flags = flags | Qt.ItemFlag.ItemIsEditable
        return flags

    def load(self, assets_path):
        self.beginResetModel()
        self._data_list = []
        self._pending_deletions = []

        path = os.path.join(assets_path, "defs", self.folder, "*.json")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]

                with open(file_path, "r") as f:
                    data = json.load(f)
                    data["name"] = name
                    data["modified"] = False
                    self._data_list.append(data)

        self.endResetModel()

    def save(self, assets_path):
        base_path = os.path.join(assets_path, "defs", self.folder)
        os.makedirs(base_path, exist_ok=True)

        # Delete pending files
        while self._pending_deletions:
            p = self._pending_deletions.pop()
            if os.path.exists(p): os.remove(p)

        # Add or modify files marked with modifed
        for row, item in enumerate(self._data_list):
            if item.get('modified'):
                path = os.path.join(base_path, f"{item['name']}.json")
                save_data = copy.deepcopy(item)
                for k in META_KEYS: save_data.pop(k, None)

                # Handle specific case for StartFiring event. Need to prune the emitter keys
                # TODO: Need a better way of handling this... maybe an override
                if self.folder == "enemy":
                    from tools.def_editor.models.emitter import EmitterModel
                    model = EmitterModel()
                    for event in save_data.get('events', []):
                        if event.get('type') == 'StartFiring':
                            model.set_data(event)
                            pruned = model.export_data()
                            event.clear()
                            event.update(pruned)
                
                with open(path, "w") as f:
                    json.dump(save_data, f, indent=2, separators=(',', ': '))
                
                item['modified'] = False
                if self.COL_MODIFIED != -1:
                    m_idx = self.index(row, self.COL_MODIFIED)
                    self.dataChanged.emit(m_idx, m_idx, [Qt.ItemDataRole.DisplayRole])

    def add(self, data):
        parent_index = QModelIndex() 
        row_count = len(self._data_list)

        # Add missing keys
        for col_info in self._COLUMN_MAP.values():
            key = col_info['key']
            col_type = col_info['type']
            if key not in data:
                data[key] = col_type()
        data['modified'] = True
        
        self.beginInsertRows(parent_index, row_count, row_count)
        self._data_list.append(data)
        self.endInsertRows()

    def move_up(self, index):
        if index.row() > 0:
            self._data_list[index.row()], self._data_list[index.row() - 1] = self._data_list[index.row() - 1], self._data_list[index.row()]
            self.dataChanged.emit(index, index)
    def move_down(self, index):
        if index.row() < len(self._data_list) - 1:
            self._data_list[index.row()], self._data_list[index.row() + 1] = self._data_list[index.row() + 1], self._data_list[index.row()]

    def isDirty(self):
        return any(item.get('modified') for item in self._data_list) or len(self._pending_deletions) > 0

    def get_by_name(self, name):
        for item in self._data_list:
            if item.get('name') == name:
                return item
        return None

    def exists(self, key):
        from tools.def_editor import defsdb
        return bool([x for x in self._data_list if x['name'] == key]) or os.path.isfile(f'{defsdb.assets_path}/defs/{self.folder}/{key}.json')