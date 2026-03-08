from PyQt6.QtWidgets import QWidget, QDataWidgetMapper, QMessageBox, QInputDialog

from ui.Scripts import Ui_pageScripts

from tools.def_editor import defsdb
from models import ScriptModel

class pageScripts(QWidget, Ui_pageScripts):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.stackedControls.setCurrentIndex(1)

        self.lvScripts.setModel(defsdb.scripts)
        self.lvScripts.setModelColumn(0)

        self.lvScripts.selectionModel().currentChanged.connect(self.index_changed)

        self.mapper = QDataWidgetMapper(self)
        self.mapper.setModel(defsdb.scripts)
        self.mapper.addMapping(self.teSource, ScriptModel.COL_SOURCE, b'plainText')

        self.mapper.setCurrentIndex(0)

        self.btnNewScript.clicked.connect(self.new_script)

    def index_changed(self, new):
        if new.isValid():
            self.stackedControls.setCurrentIndex(1)
            self.mapper.setCurrentIndex(new.row())
        else:
            self.stackedControls.setCurrentIndex(0)

    def new_script(self):
        val, res = QInputDialog.getText(self, "Add script...", "Name")
        val = val.strip()

        if res:
            if defsdb.scripts.exists(val):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                defsdb.scripts.add(val)