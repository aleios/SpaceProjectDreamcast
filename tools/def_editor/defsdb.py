from PyQt6.QtCore import QAbstractTableModel, Qt, QModelIndex
import json
from dataclasses import dataclass, asdict, field
from models import EnemyModel, ProjectileModel, LevelsModel, PlaylistModel, PlayerModel, AnimationModel

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
        
        # Prune emitter keys that we don't need.
        from tools.def_editor.models.emitter import EmitterModel
        model = EmitterModel()
        for weapon in d.get('weapons', []):
            for emitter in weapon.get('emitters', []):
                model.set_data(emitter)
                pruned = model.export_data()
                emitter.clear()
                emitter.update(pruned)

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


# Def models
projectile_defs = ProjectileModel()
enemy_defs = EnemyModel()
player_def = PlayerModel()

# Game settings models
game_settings = GameSettingsData()
game_settings_model = GameSettingsModel(game_settings)
playlist_model = PlaylistModel(game_settings)

# Animation models
animations = AnimationModel()

levels = LevelsModel()

assets_path = ""

def reload_defs():
    projectile_defs.load(assets_path)
    enemy_defs.load(assets_path)

    animations.load(assets_path)
    levels.load(assets_path)

    try:
        with open(assets_path + "/settings.json", "r") as f:
            data = json.load(f)
            game_settings_model.beginResetModel()
            #weapons_model.beginResetModel()
            playlist_model.beginResetModel()
            game_settings.from_dict(data)
            game_settings_model.endResetModel()
            #weapons_model.endResetModel()
            playlist_model.endResetModel()
    except IOError:
        print("Loading default game settings")

    player_def.load(assets_path)

    print("Defs reloaded")

def save_pending_defs():
    projectile_defs.save(assets_path)
    enemy_defs.save(assets_path)
    animations.save(assets_path)
    levels.save(assets_path)

    player_def.save(assets_path)

    if game_settings.modified:
        path = assets_path + "/settings.json"
        with open(path, "w") as f:
            json.dump(game_settings.to_dict(), f, indent=2, separators=(',', ': '))
        game_settings.modified = False

def is_dirty():
    return projectile_defs.isDirty() or enemy_defs.isDirty() or animations.isDirty() or game_settings.modified