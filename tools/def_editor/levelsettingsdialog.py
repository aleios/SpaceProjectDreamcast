from PyQt6.QtWidgets import QDialog, QWidget

from tools.def_editor import defsdb
from ui.LevelSettingsDialog import Ui_levelSettingsDialog

class LevelSettingsDialog(QDialog, Ui_levelSettingsDialog):
    def __init__(self, current_settings, parent=None):
        super().__init__(parent)
        self.setupUi(self)

        self.cbInitialMusic.setModel(defsdb.music)
        idx = self.cbInitialMusic.findText(current_settings.get('initial_music', ''))
        if idx >= 0:
            self.cbInitialMusic.setCurrentIndex(idx)
        self.sbScrollSpeed.setValue(current_settings.get('scroll_speed', 0))

        self.cbStarfieldEnabled.setChecked(current_settings.get('starfield_enabled', True))
        self.sbStarfieldSpeed.setValue(current_settings.get('starfield_speed', 0.08))