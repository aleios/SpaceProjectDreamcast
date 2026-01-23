from PyQt6.QtWidgets import QDialog, QWidget
from ui.LevelSettingsDialog import Ui_levelSettingsDialog

class LevelSettingsDialog(QDialog, Ui_levelSettingsDialog):
    def __init__(self, current_settings, parent=None):
        super().__init__(parent)
        self.setupUi(self)

        self.tbInitialMusic.setText(current_settings.get('initial_music', ''))
        self.sbScrollSpeed.setValue(current_settings.get('scroll_speed', 0))