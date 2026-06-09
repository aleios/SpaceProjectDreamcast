from PyQt6.QtWidgets import QDialog
from ui.ScriptEditorDialog import Ui_scriptEditorDialog


class ScriptEditorDialog(QDialog, Ui_scriptEditorDialog):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

    def set_script(self, script):
        self.teScript.setPlainText(script)
    def get_script(self):
        return self.teScript.toPlainText()