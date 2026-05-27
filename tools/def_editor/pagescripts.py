from PyQt6.QtCore import Qt
from PyQt6.QtGui import QAction
from PyQt6.QtWidgets import QWidget, QDataWidgetMapper, QMessageBox, QInputDialog, QPushButton, QMenu

from ui.Scripts import Ui_pageScripts

from tools.def_editor import defsdb
from models import ScriptModel

class pageScripts(QWidget, Ui_pageScripts):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.stackedControls.setCurrentIndex(0)

        self.lvScripts.setModel(defsdb.scripts)
        self.lvScripts.setModelColumn(0)
        self.lvScripts.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.lvScripts.customContextMenuRequested.connect(self.ctx_menu)

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

    def select_script_by_index(self, index):
        idx = defsdb.scripts.index(index, 0)
        if idx.isValid():
            self.lvScripts.setCurrentIndex(idx)

    def new_script(self):
        val, res = QInputDialog.getText(self, "Add script...", "Name")
        val = val.strip()

        if res:
            if defsdb.scripts.exists(val):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                defsdb.scripts.add(val)

    def ctx_menu(self, pt):
        idx = self.lvScripts.indexAt(pt)
        if not idx.isValid():
            return

        menu = QMenu()
        delete_action = QAction("Delete")
        delete_action.triggered.connect(lambda : defsdb.scripts.remove(idx))
        menu.addAction(delete_action)
        menu.exec(self.lvScripts.mapToGlobal(pt))