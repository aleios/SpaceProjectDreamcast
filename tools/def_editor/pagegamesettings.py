from PyQt6.QtWidgets import QWidget, QDataWidgetMapper
from PyQt6.QtCore import Qt, QModelIndex
from tools.def_editor.ui.Gamesettings import Ui_pageGameSettings
from tools.def_editor.projectiledialog import ProjectileDialog

from tools.def_editor import defsdb

class pageGameSettings(QWidget, Ui_pageGameSettings):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        #self.lstPlayerProj.
        # self.weapons_model = defsdb.player_def.make_weapons_model()
        #
        # self.weapons_model.rowsAboutToBeRemoved.connect(self.model_rows_removed)
        # self.weapons_model.modelReset.connect(self.model_reset)

        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(defsdb.game_settings_model)
        self.fieldMapper.setOrientation(Qt.Orientation.Horizontal)
        self.fieldMapper.setSubmitPolicy(QDataWidgetMapper.SubmitPolicy.ManualSubmit)

        # Explicitly map to the 'value' property of the spinboxes.
        self.fieldMapper.addMapping(self.sbMaxLives, 0, b"value")
        self.fieldMapper.addMapping(self.sbMaxHealth, 1, b"value")
        self.fieldMapper.setCurrentIndex(0)

        self.sbMaxLives.valueChanged.connect(self.fieldMapper.submit)
        self.sbMaxHealth.valueChanged.connect(self.fieldMapper.submit)

        self.lvLevelPlaylist.setModel(defsdb.playlist_model)

        # Connect reset signals for the model.
        defsdb.game_settings_model.modelReset.connect(self.model_reset)

        self.btnAddLevel.clicked.connect(self.add_level)
        self.btnRemoveLevel.clicked.connect(self.remove_level)
        self.btnMoveLevelUp.clicked.connect(self.move_level_up)
        self.btnMoveLevelDown.clicked.connect(self.move_level_down)

    def model_reset(self):
        self.fieldMapper.setCurrentIndex(0)

    def add_level(self):
        from PyQt6.QtWidgets import QInputDialog
        levels = [l['name'] for l in defsdb.levels.levels]
        if not levels:
            return
        
        level, ok = QInputDialog.getItem(self, "Add Level", "Select Level:", levels, 0, False)
        if ok and level:
            defsdb.playlist_model.add(level)

    def remove_level(self):
        idx = self.lvLevelPlaylist.currentIndex()
        if idx.isValid():
            defsdb.playlist_model.remove(idx.row())
            self.lvLevelPlaylist.setCurrentIndex(QModelIndex())

    def move_level_up(self):
        idx = self.lvLevelPlaylist.currentIndex()
        if idx.isValid():
            row = idx.row()
            defsdb.playlist_model.shift_up(row)
            if row > 0:
                self.lvLevelPlaylist.setCurrentIndex(defsdb.playlist_model.index(row - 1, 0))

    def move_level_down(self):
        idx = self.lvLevelPlaylist.currentIndex()
        if idx.isValid():
            row = idx.row()
            defsdb.playlist_model.shift_down(row)
            if row < defsdb.playlist_model.rowCount() - 1:
                self.lvLevelPlaylist.setCurrentIndex(defsdb.playlist_model.index(row + 1, 0))

    def model_rows_removed(self, parent, first, last):
        print('removed: ', parent, first, last)