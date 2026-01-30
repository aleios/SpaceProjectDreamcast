import os
import json
from PyQt6.QtWidgets import QMainWindow, QMessageBox
from PyQt6.QtCore import pyqtSignal
from tools.def_editor.ui.Mainwindow import Ui_MainWindow
from tools.def_editor.pathsdialog import PathsDialog

from tools.def_editor import defsdb

class MainWindow(QMainWindow, Ui_MainWindow):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        if os.path.exists("settings.json"):
            with open("settings.json", "r") as settings_file:
                jx = json.load(settings_file)
                assets_path = jx.get("assets_path", "")

                if assets_path:
                    defsdb.assets_path = assets_path
                    defsdb.reload_defs()
        else:
            self.tabWidget.hide()
            self.open_paths_dialog()

        self.setupUi(self)
        self.actionPaths.triggered.connect(self.open_paths_dialog)
        self.btnSave.clicked.connect(self.save)

    def open_paths_dialog(self):
        dlg = PathsDialog(self)
        res = dlg.exec()

        if res:
            defsdb.assets_path = dlg.tbPaths.text()

            if os.path.isdir(defsdb.assets_path) and os.access(defsdb.assets_path, os.R_OK) and os.access(defsdb.assets_path, os.W_OK):
                
                with open("settings.json", "w") as settings_file:
                    json.dump({ "assets_path": defsdb.assets_path }, settings_file, indent=2, separators=(',', ': '))

                defsdb.reload_defs()

                # TODO: validate its a proper path before enabling.
                if self.tabWidget.isHidden():
                    self.tabWidget.show()
            else:
                QMessageBox.critical(self, "Error", "Selected assets path does not exist or has insufficent permissions.")
                self.open_paths_dialog()

    def save(self):
        res = QMessageBox.warning(self, "Warning", "Warning: Are you sure you want to save? All files will be overwritten on disk.", QMessageBox.StandardButton.Ok | QMessageBox.StandardButton.Cancel, QMessageBox.StandardButton.Cancel)

        if res == QMessageBox.StandardButton.Ok:
            defsdb.save_pending_defs()

    def closeEvent(self, a0):
        return super().closeEvent(a0)