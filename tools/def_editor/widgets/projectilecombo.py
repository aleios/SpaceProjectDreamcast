from PyQt6.QtWidgets import QComboBox
from tools.def_editor import defsdb

class ProjectileCombo(QComboBox):
    def __init__(self, parent = None):
        super().__init__(parent)

        #self.setModel(defsdb.projectile_defs)