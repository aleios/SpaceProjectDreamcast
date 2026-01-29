from PyQt6.QtCore import QAbstractTableModel, Qt, QModelIndex, QAbstractListModel, QMimeData
import json
import os
import glob

class LevelEventsModel(QAbstractListModel):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.events = []

    def set_events(self, events):
        self.beginResetModel()
        self.events = events
        self.endResetModel()

    def add_event(self, event):
        self.beginInsertRows(QModelIndex(), len(self.events), len(self.events))
        self.events.append(event)
        self.endInsertRows()

    def update_event(self, index, event):
        if 0 <= index < len(self.events):
            self.events[index] = event
            self.dataChanged.emit(self.index(index), self.index(index))
            return True
        return False

    def remove_event(self, index):
        if 0 <= index < len(self.events):
            self.beginRemoveRows(QModelIndex(), index, index)
            self.events.pop(index)
            self.endRemoveRows()
            return True
        return False

    def rowCount(self, parent=QModelIndex()):
        return len(self.events)

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < len(self.events)):
            return None

        event_wrapper = self.events[index.row()]
        event_data = event_wrapper.get('event', {})

        if role == Qt.ItemDataRole.DisplayRole:
            etype = event_data.get('type', 'unknown')
            if etype == 'spawn':
                return f"spawn: {event_data.get('def', '???')}"
            return etype

        return None

class LevelsModel(QAbstractTableModel):
    COL_NAME, COL_MUSIC, COL_SPEED, COL_MODIFIED, COL_EVENTS = range(5)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.levels = []

    def rowCount(self, parent=QModelIndex()):
        return len(self.levels)

    def columnCount(self, parent=QModelIndex()):
        return 5 # Name, Music, Speed, Modified, Events

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < len(self.levels)):
            return None

        item = self.levels[index.row()]
        col = index.column()

        if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            if col == self.COL_NAME: return item['name']
            if col == self.COL_MUSIC: return item['initial_music']
            if col == self.COL_SPEED: return item['scroll_speed']
            if col == self.COL_MODIFIED: return item['modified']
            if col == self.COL_EVENTS: return item['events']
        return None

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            row = index.row()
            col = index.column()
            item = self.levels[row]

            if col == self.COL_MUSIC:
                item['initial_music'] = value
            elif col == self.COL_SPEED:
                item['scroll_speed'] = float(value)
            elif col == self.COL_NAME:
                item['name'] = value
            elif col == self.COL_MODIFIED:
                item['modified'] = bool(value)
                self.dataChanged.emit(index, index, [role])
                return True
            else:
                return False

            item['modified'] = True
            self.dataChanged.emit(index, index, [role])
            # Notify modified column too
            mod_idx = self.index(row, self.COL_MODIFIED)
            self.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])
            return True
        return False

    def flags(self, index):
        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags

        flags = Qt.ItemFlag.ItemIsEnabled | Qt.ItemFlag.ItemIsSelectable
        if index.column() == self.COL_MUSIC:
            flags |= Qt.ItemFlag.ItemIsEditable
        return flags

    def add(self, name):
        self.beginInsertRows(QModelIndex(), len(self.levels), len(self.levels))
        self.levels.append({
            "name": name,
            "initial_music": "hope",
            "scroll_speed": 0.2,
            "modified": True,
            "events": []
        })
        self.endInsertRows()

    def load(self, assets_path):
        self.beginResetModel()
        self.levels = []
        path = os.path.join(assets_path, "levels", "*.json")
        for file_path in glob.glob(path):
            name = os.path.splitext(os.path.basename(file_path))[0]
            with open(file_path, "r") as f:
                try:
                    data = json.load(f)
                    self.levels.append({
                        "name": name,
                        "initial_music": data.get("initial_music", ""),
                        "scroll_speed": data.get("scroll_speed", 0.2),
                        "events": data.get("events", []),
                        "modified": False
                    })
                except json.JSONDecodeError:
                    print(f"Failed to load level: {file_path}")
        self.endResetModel()

    def gather_preloads(self, events):
        preloads = {
            "enemies": set(),
            "projectiles": set()
        }
        for event in events:
            event_data = event.get('event', {})
            if event_data.get('type', '') == 'spawn':
                enemy_name = event_data.get('def', '')
                if enemy_name:
                    preloads['enemies'].add(enemy_name)
                    # Get enemy event stack, extract out the projectile defs used.
                    from tools.def_editor import defsdb
                    enemy_data = defsdb.enemy_defs.get_by_name(enemy_name)
                    if enemy_data:
                        enemy_events = enemy_data.get('events', [])
                        for e_event in enemy_events:
                            if e_event.get('type') == 'StartFiring':
                                projectile_name = e_event.get('projectile')
                                if projectile_name:
                                    preloads['projectiles'].add(projectile_name)

        # Convert to lists for json
        return {
            "enemies": sorted(list(preloads['enemies'])),
            "projectiles": sorted(list(preloads['projectiles']))
        }

    def save(self, assets_path):
        base_path = os.path.join(assets_path, "levels")
        os.makedirs(base_path, exist_ok=True)
        for row, item in enumerate(self.levels):
            if item['modified']:
                print(f"Saving {item['name']}")
                file_path = os.path.join(base_path, f"{item['name']}.json")
                sorted_events = sorted(item['events'], key=lambda x: x['pos'][1])
                preloads = self.gather_preloads(sorted_events)
                out_data = {
                    "initial_music": item['initial_music'],
                    "scroll_speed": item['scroll_speed'],
                    "preloads": preloads,
                    "events": sorted_events
                }
                with open(file_path, "w") as f:
                    json.dump(out_data, f, indent=2, separators=(',', ': '))
                item['modified'] = False
                mod_idx = self.index(row, self.COL_MODIFIED)
                self.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])

    def exists(self, key):
        from tools.def_editor import defsdb
        return bool([x for x in self.levels if x['name'] == key]) or os.path.isfile(
            defsdb.assets_path + "/levels/" + key + ".json")
