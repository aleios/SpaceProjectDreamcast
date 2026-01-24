from PyQt6.QtCore import QAbstractTableModel, Qt, QModelIndex, QAbstractListModel, QMimeData
import os
import glob
import json
from dataclasses import dataclass, asdict, field
from models import EnemyModel, ProjectileModel, LevelsModel
from PIL import Image

#
# -- Animation Model
#
class ClipFramesModel(QAbstractTableModel):
    # COL_DISPLAY is for list views showing [x, y, w, h]
    COL_DISPLAY, COL_X, COL_Y, COL_W, COL_H = range(5)

    def __init__(self, clip_dict, parent_clip_model):
        super().__init__()
        self.clip_dict = clip_dict
        self.frames = clip_dict.get('frames', [])
        self.parent_clip_model = parent_clip_model

    def rowCount(self, parent=QModelIndex()):
        return len(self.frames)

    def columnCount(self, parent=QModelIndex()):
        return 5

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < len(self.frames)):
            return None

        row_data = self.frames[index.row()]
        col = index.column()

        if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            # Format for list views if requesting display.
            if col == self.COL_DISPLAY:
                return f"[{row_data[0]}, {row_data[1]}, {row_data[2]}, {row_data[3]}]"
            
            # Otherwise grab the internal values
            if col == self.COL_X: return row_data[0]
            if col == self.COL_Y: return row_data[1]
            if col == self.COL_W: return row_data[2]
            if col == self.COL_H: return row_data[3]

        return None

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            row = index.row()
            col = index.column()
            
            try:
                val = float(value)
                if col == self.COL_X: self.frames[row][0] = val
                elif col == self.COL_Y: self.frames[row][1] = val
                elif col == self.COL_W: self.frames[row][2] = val
                elif col == self.COL_H: self.frames[row][3] = val
                else: return False

                # Update list view str
                display_idx = self.index(row, self.COL_DISPLAY)
                self.dataChanged.emit(display_idx, display_idx, [Qt.ItemDataRole.DisplayRole])
                
                # update mapper col
                self.dataChanged.emit(index, index, [role])
                
                # Notify parent of vhanges.
                self.parent_clip_model.notify_changed()
                return True
            except (ValueError, IndexError):
                return False
        return False
    
    def add(self):
        self.beginInsertRows(QModelIndex(), len(self.frames), len(self.frames))
        self.frames.append([0.0,0.0,0.0,0.0])
        self.endInsertRows()

    def remove(self, row):
        if 0 <= row < len(self.frames):
            self.beginRemoveRows(QModelIndex(), row, row)
            self.frames.pop(row)
            self.endRemoveRows()

    def shift_up(self, row):
        if row > 0:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)
            self.frames[row], self.frames[row - 1] = self.frames[row - 1], self.frames[row]
            self.endMoveRows()

    def shift_down(self, row):
        if row < len(self.frames) - 1:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2)
            self.frames[row], self.frames[row + 1] = self.frames[row + 1], self.frames[row]
            self.endMoveRows()

class ClipListModel(QAbstractTableModel):
    COL_NAME, COL_FPS, COL_LOOPMODE, COL_ORIGIN_X, COL_ORIGIN_Y = range(5)

    def __init__(self, parent_model, parent_row, data_override=None, include_empty=False):
        super().__init__()
        self.parent_model = parent_model
        self.parent_row = parent_row
        self.include_empty = include_empty

        if data_override is not None:
            self.clips = data_override
        else:
            self.clips = parent_model._data_list[parent_row]['clips']

    def rowCount(self, parent=QModelIndex()):
        return len(self.clips) + (1 if self.include_empty else 0)
    
    def columnCount(self, parent=QModelIndex()):
        return 5

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid():
            return None
        
        row = index.row()
        if self.include_empty:
            if row < 0 or row > len(self.clips):
                return None
            if row == 0:
                if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
                    if index.column() == self.COL_NAME:
                        return ""
                return None
            data_row = row - 1
        else:
            if row < 0 or row >= len(self.clips):
                return None
            data_row = row

        if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            col = index.column()
            clip = self.clips[data_row]
            
            if col == self.COL_NAME:
                return clip.get('name', "")
            if col == self.COL_FPS:
                return clip.get('fps', 0.0)
            if col == self.COL_LOOPMODE:
                return clip.get('loop_mode', 0)
            if col == self.COL_ORIGIN_X:
                return clip.get('origin', [0,0])[0]
            if col == self.COL_ORIGIN_Y:
                return clip.get('origin', [0,0])[1]
                
        return None

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            row = index.row()
            if self.include_empty:
                if row == 0:
                    return False
                data_row = row - 1
            else:
                data_row = row

            col = index.column()
            clip = self.clips[data_row]

            if col == self.COL_NAME:
                clip['name'] = str(value)
            elif col == self.COL_FPS:
                clip['fps'] = float(value)
            elif col == self.COL_LOOPMODE:
                clip['loop_mode'] = int(value)
            elif col == self.COL_ORIGIN_X:
                clip['origin'][0] = float(value)
            elif col == self.COL_ORIGIN_Y:
                clip['origin'][1] = float(value)
            else:
                return False

            self.dataChanged.emit(index, index, [role, Qt.ItemDataRole.DisplayRole])
            self.notify_changed()
            return True
        return False

    def flags(self, index):
        if self.include_empty and index.row() == 0:
            return super().flags(index)
        return super().flags(index) | Qt.ItemFlag.ItemIsEditable

    def get_clip_data(self, row):
        if self.include_empty:
            if row == 0:
                return None
            return self.clips[row - 1]
        return self.clips[row]

    def notify_changed(self):
        idx = self.parent_model.index(self.parent_row, AnimationModel.COL_MODIFIED)
        self.parent_model.setData(idx, True)

    def add(self, name):
        self.beginInsertRows(QModelIndex(), len(self.clips), len(self.clips))
        global_origin = self.parent_model.get_global_origin(self.parent_row)
        self.clips.append({ "name": name, "fps": 0.0, "loop_mode": 0, "frames": [], "origin": global_origin })
        self.endInsertRows()
        self.notify_changed()

class AnimationModel(QAbstractTableModel):
    COL_NAME, COL_MODIFIED, COL_TEXTURE, COL_ORIGIN, COL_ORIGIN_X, COL_ORIGIN_Y, COL_CLIPS = range(7)

    def __init__(self):
        super().__init__()
        self._data_list = []
        self._pending_deletions = []
        self.folder = "animations"

    def rowCount(self, parent=QModelIndex()):
        return len(self._data_list)
    
    def columnCount(self, parent=QModelIndex()):
        return 7

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or not (0 <= index.row() < len(self._data_list)):
            return None
        
        item = self._data_list[index.row()]
        col = index.column()

        if role in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            if col == self.COL_NAME: return item['name']
            if col == self.COL_MODIFIED: return item['modified']
            if col == self.COL_TEXTURE: return item['texture']
            if col == self.COL_ORIGIN_X:
                return item['origin'][0] if len(item['origin']) > 0 else 0
            if col == self.COL_ORIGIN_Y:
                return item['origin'][1] if len(item['origin']) > 1 else 0
            if col == self.COL_CLIPS: return item['clips'] # Returns the list of dicts
        return None

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if not index.isValid() or role != Qt.ItemDataRole.EditRole:
            return False

        row = index.row()
        col = index.column()
        item = self._data_list[row]
        changed = False

        if col == self.COL_ORIGIN_X:
            item['origin'][0] = float(value)
            changed = True
        elif col == self.COL_ORIGIN_Y:
            item['origin'][1] = float(value)
            changed = True
        elif col == self.COL_TEXTURE:
            item['texture'] = value
            changed = True
        elif col == self.COL_NAME:
            item['name'] = value
            changed = True
        elif col == self.COL_MODIFIED:
            item['modified'] = bool(value)
            self.dataChanged.emit(index, index, [role])
            return True

        if changed:
            self._mark_dirty(row, index)
            return True
            
        return False

    def get_global_origin(self, row: int):
        x = self.data(self.index(row, self.COL_ORIGIN_X), Qt.ItemDataRole.EditRole)
        y = self.data(self.index(row, self.COL_ORIGIN_Y), Qt.ItemDataRole.EditRole)
        return [x, y]

    @classmethod
    def _normalize_clip_data(cls, name, v):
        if not isinstance(v, dict):
            v = {}
        clip = { "name": name, **v }
        clip.setdefault("fps", 0.0)
        clip.setdefault("loop_mode", 0)
        clip.setdefault("frames", [])
        clip.setdefault("origin", [0.0,0.0])
        return clip

    def load(self):
        self.beginResetModel()
        self._data_list = []
        path = os.path.join(assets_path, self.folder, "*.json")
        for file_path in glob.glob(path):
            if os.path.isfile(file_path):
                name = os.path.splitext(os.path.basename(file_path))[0]
                with open(file_path, "r") as f:
                    data = json.load(f)
                    mapped_data = {
                        "name": name, 
                        "modified": False,
                        "texture": "",
                        "origin": [0, 0],
                        "clips": []
                    }
                    if '_meta' in data:
                        meta = data["_meta"]
                        mapped_data['texture'] = meta.get("texture", "")
                        mapped_data['origin'] = meta.get("origin", [0, 0])
                    
                    for k, v in data.items():
                        if k == '_meta': continue
                        mapped_data["clips"].append(self._normalize_clip_data(k, v))
                    
                    self._data_list.append(mapped_data)
        self.endResetModel()

    def get_clip_list_model(self, row, include_empty=False):
        return ClipListModel(self, row, include_empty=include_empty)

    def save(self, assets_path):
        base_path = os.path.join(assets_path, self.folder)
        os.makedirs(base_path, exist_ok=True)
        for item in self._data_list:
            if item['modified']:
                tex = item.get('texture', None)

                # TODO: Error message... or maybe just 0 out the atlas w/h
                atlas_width = 0
                atlas_height = 0
                if tex:
                    # Get atlas_width and atlas_height from texture dimensions
                    try:
                        tex_path = os.path.join(assets_path, f"sprites/{tex}.png")
                        print(tex_path)
                        with Image.open(tex_path) as im:
                            atlas_width = im.width
                            atlas_height = im.height
                    except:
                        pass
                else:
                    pass
                    # Spawn warning message box

                # Reconstruct JSON
                out = {
                    "_meta": {
                        "texture": item['texture'],
                        "origin": item['origin'],
                        "atlas_width": atlas_width,
                        "atlas_height": atlas_height
                    }
                }
                # Add clips back as keys
                for clip in item['clips']:
                    clip_copy = dict(clip)
                    name = clip_copy.pop('name')
                    out[name] = clip_copy
                
                file_path = os.path.join(base_path, f"{item['name']}.json")
                with open(file_path, "w") as f:
                    json.dump(out, f, indent=2, separators=(',', ': '))
                item['modified'] = False
        self.layoutChanged.emit()

    def _mark_dirty(self, row, index):
        if not self._data_list[row]['modified']:
            self._data_list[row]['modified'] = True
            mod_idx = self.index(row, self.COL_MODIFIED)
            self.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])
        
        self.dataChanged.emit(index, index, [Qt.ItemDataRole.EditRole, Qt.ItemDataRole.DisplayRole])

    def isDirty(self):
        return any(item.get('modified') for item in self._data_list) or len(self._pending_deletions) > 0
    
    def add(self, name):
        self.beginInsertRows(QModelIndex(), len(self._data_list), len(self._data_list))
        self._data_list.append({
            "name": name,
            "modified": True,
            "texture": "",
            "origin": [0.0,0.0],
            "clips": []
        })
        self.endInsertRows()

#
# -- Game settings models --
#
@dataclass
class GameSettingsData:
    max_health: int = 10
    max_lives: int = 3
    player_speed: float = 0.2
    weapons: list = field(default_factory=list)
    playlist: list = field(default_factory=list)
    modified: bool = True

    def to_dict(self):
        d = asdict(self)
        d.pop('modified', None)
        return d
    
    def from_dict(self, data: dict):
        self.max_health = data.get("max_health", self.max_health)
        self.max_lives = data.get("max_lives", self.max_lives)
        self.player_speed = data.get("player_speed", self.player_speed)
        self.weapons = list(data.get("weapons", []))
        self.playlist = list(data.get("playlist", []))
        self.modified = False

class GameSettingsModel(QAbstractTableModel):
    def __init__(self, data_obj: GameSettingsData, parent=None):
        super().__init__(parent)
        self.data_obj = data_obj
        self.fields = ["max_lives", "max_health", "player_speed"]

    def rowCount(self, parent=QModelIndex()):
        return 1

    def columnCount(self, parent=QModelIndex()):
        return len(self.fields)

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        return getattr(self.data_obj, self.fields[index.column()])

    def flags(self, index):
        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags
        return Qt.ItemFlag.ItemIsEnabled | Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsEditable

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            field_name = self.fields[index.column()]
            attr = getattr(self.data_obj, field_name)
            target_type = type(attr)
            new_val = target_type(value)
            if attr != new_val:
                setattr(self.data_obj, field_name, new_val)
                self.data_obj.modified = True
                self.dataChanged.emit(index, index, [Qt.ItemDataRole.EditRole])
            return True
        return False

class WeaponSetModel(QAbstractTableModel):
    mimeType = "application/x-weapon-set-item"

    def __init__(self, data_obj: GameSettingsData, parent=None):
        super().__init__(parent)
        self.data_obj = data_obj

    def rowCount(self, parent=QModelIndex()):
        return len(self.data_obj.weapons)

    def columnCount(self, parent=QModelIndex()):
        return 1

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid() or role not in (Qt.ItemDataRole.DisplayRole, Qt.ItemDataRole.EditRole):
            return None
        
        item = self.data_obj.weapons[index.row()]
        if role == Qt.ItemDataRole.DisplayRole:
            if isinstance(item, dict):
                return item.get('name', 'Unnamed')
            return str(item)
        
        return item

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if index.isValid() and role == Qt.ItemDataRole.EditRole:
            if self.data_obj.weapons[index.row()] != value:
                self.data_obj.weapons[index.row()] = value
                self.data_obj.modified = True
                self.dataChanged.emit(index, index, [role, Qt.ItemDataRole.DisplayRole])
            return True
        return False

    def supportedDropActions(self):
        return Qt.DropAction.MoveAction

    def flags(self, index):
        return super().flags(index) | Qt.ItemFlag.ItemIsDragEnabled | Qt.ItemFlag.ItemIsDropEnabled

    def add(self, weapon_set):
        self.beginInsertRows(QModelIndex(), len(self.data_obj.weapons), len(self.data_obj.weapons))
        self.data_obj.weapons.append(weapon_set)
        self.data_obj.modified = True
        self.endInsertRows()

    def remove(self, row):
        if 0 <= row < len(self.data_obj.weapons):
            self.beginRemoveRows(QModelIndex(), row, row)
            self.data_obj.weapons.pop(row)
            self.data_obj.modified = True
            self.endRemoveRows()

    def shift_up(self, row):
        if row > 0:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row - 1)
            self.data_obj.weapons[row], self.data_obj.weapons[row - 1] = self.data_obj.weapons[row - 1], self.data_obj.weapons[row]
            self.data_obj.modified = True
            self.endMoveRows()

    def shift_down(self, row):
        if row < len(self.data_obj.weapons) - 1:
            self.beginMoveRows(QModelIndex(), row, row, QModelIndex(), row + 2)
            self.data_obj.weapons[row], self.data_obj.weapons[row + 1] = self.data_obj.weapons[row + 1], self.data_obj.weapons[row]
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

        item = self.data_obj.weapons.pop(src_row)
        insert_at = dest_row if dest_row <= src_row else dest_row - 1
        self.data_obj.weapons.insert(insert_at, item)
        self.data_obj.modified = True

        self.endMoveRows()
        return True


class PlaylistModel(QAbstractTableModel):
    mimeType = "application/x-playlist-item"

    def __init__(self, data_obj: GameSettingsData, parent=None):
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


# Def models
projectile_defs = ProjectileModel()
enemy_defs = EnemyModel()

# Game settings models
game_settings = GameSettingsData()
game_settings_model = GameSettingsModel(game_settings)
weapons_model = WeaponSetModel(game_settings)
playlist_model = PlaylistModel(game_settings)

# Animation models
animations = AnimationModel()

levels = LevelsModel()

pending_removals = []

assets_path = ""

def reload_defs():
    projectile_defs.load(assets_path)
    enemy_defs.load(assets_path)

    animations.load()
    levels.load(assets_path)

    try:
        with open(assets_path + "/settings.json", "r") as f:
            data = json.load(f)
            game_settings_model.beginResetModel()
            weapons_model.beginResetModel()
            playlist_model.beginResetModel()
            game_settings.from_dict(data)
            game_settings_model.endResetModel()
            weapons_model.endResetModel()
            playlist_model.endResetModel()
    except IOError:
        print("Loading default game settings")

    print("Defs reloaded")

def save_pending_defs():
    projectile_defs.save(assets_path)
    enemy_defs.save(assets_path)
    animations.save(assets_path)
    levels.save(assets_path)

    if game_settings.modified:
        path = assets_path + "/settings.json"
        print("Saving", path)
        with open(path, "w") as f:
            json.dump(game_settings.to_dict(), f, indent=2, separators=(',', ': '))
        game_settings.modified = False

def is_dirty():
    return projectile_defs.isDirty() or enemy_defs.isDirty() or animations.isDirty() or game_settings.modified