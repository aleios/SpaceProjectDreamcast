from PyQt6.QtWidgets import QDialog, QWidget
from ui.EditorSettingsDialog import Ui_editorSettingsDialog

class EditorSettingsDialog(QDialog, Ui_editorSettingsDialog):
    def __init__(self, current_settings, parent=None):
        super().__init__(parent)
        self.setupUi(self)

        self.sbGridWidth.setValue(current_settings.get('grid_width', 8))
        self.sbGridHeight.setValue(current_settings.get('grid_height', 8))
        self.cbShowGrid.setChecked(current_settings.get('grid_enabled', True))
        self.cbSnapGrid.setChecked(current_settings.get('grid_snap', True))