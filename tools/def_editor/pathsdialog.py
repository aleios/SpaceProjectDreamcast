from PyQt6.QtWidgets import QDialog, QFileDialog
from ui.Pathsdialog import Ui_dlgPaths
import os

class PathsDialog(QDialog, Ui_dlgPaths):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.btnBrowse.clicked.connect(self.browse_clicked)

    def browse_clicked(self):
        dir = QFileDialog.getExistingDirectory(self, "Assets path", os.path.curdir, QFileDialog.Option.ShowDirsOnly | QFileDialog.Option.DontResolveSymlinks)

        if dir:
            self.tbPaths.setText(dir)