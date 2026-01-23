from PyQt6.QtWidgets import QWidget, QInputDialog, QMessageBox, QDataWidgetMapper
from PyQt6.QtCore import Qt

from ui.Projectiles import Ui_pageProjectiles

from tools.def_editor import defsdb
import os

class pageProjectiles(QWidget, Ui_pageProjectiles):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.lvProjectiles.setModel(defsdb.projectile_defs)
        self.lvProjectiles.selectionModel().currentChanged.connect(self.selection_changed)

        self.lvProjectiles.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.lvProjectiles.customContextMenuRequested.connect(self.ctx_menu)

        self.btnNewProjectile.pressed.connect(self.add_projectile)

        self.controlStack.setCurrentIndex(0)
        
        # Set up mappings
        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(defsdb.projectile_defs)
        self.fieldMapper.setOrientation(Qt.Orientation.Horizontal)

        self.fieldMapper.addMapping(self.tbProjTexture, defsdb.ProjectileModel.COL_TEXTURE)
        self.fieldMapper.addMapping(self.tbProjAnimation, defsdb.ProjectileModel.COL_ANIM)
        self.fieldMapper.addMapping(self.tbProjAnimationKey, defsdb.ProjectileModel.COL_ANIM_KEY)
        self.fieldMapper.addMapping(self.sbDamage, defsdb.ProjectileModel.COL_DAMAGE)

    def add_projectile(self):
        val, res = QInputDialog.getText(self, "Add projectile...", "Name")

        if res:
            matches = [x for x in defsdb.projectile_defs._data_list if x['name'] == val]
            if matches or os.path.isfile(defsdb.assets_path + "/defs/projectile/" + val + ".json"):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                defsdb.projectile_defs.add({ "name": val })
                # TODO: Select new entry

    def selection_changed(self, new, prev):
        if new.isValid():
            print(defsdb.projectile_defs._data_list[new.row()])

            self.fieldMapper.setCurrentIndex(new.row())
            self.controlStack.setCurrentIndex(1)
        else:
            self.controlStack.setCurrentIndex(0)

    def ctx_menu(self, p):
        print(self.lvProjectiles.indexAt(p).isValid())