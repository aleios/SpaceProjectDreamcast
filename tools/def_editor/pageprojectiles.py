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
        self.fieldMapper.addMapping(self.sbDamage, defsdb.ProjectileModel.COL_DAMAGE)
        self.fieldMapper.addMapping(self.cboxRotates, defsdb.ProjectileModel.COL_ROTATES)

        # Initial indices
        self.fieldMapper.setCurrentIndex(0)
        self.cbAnimation.setCurrentIndex(0)

        self.cbAnimation.setModel(defsdb.animations)
        self.cbAnimation.currentIndexChanged.connect(self.on_animation_changed)
        self.cbAnimationClip.currentIndexChanged.connect(self.on_animation_clip_changed)

    def on_animation_changed(self, index):
        if index < 0 or self.cbAnimation.signalsBlocked():
            return

        # Set clips list
        model = defsdb.animations.get_clip_list_model(index)
        self.cbAnimationClip.setModel(model)

        # Get projectile index
        proj_idx = self.lvProjectiles.currentIndex()
        if proj_idx.isValid():
            defsdb.projectile_defs.set_animation(proj_idx.row(), self.cbAnimation.currentText())

    def on_animation_clip_changed(self, index):
        if index < 0 or self.cbAnimationClip.signalsBlocked():
            return

        proj_idx = self.lvProjectiles.currentIndex()
        if proj_idx.isValid():
            defsdb.projectile_defs.set_animation_clip(proj_idx.row(), self.cbAnimationClip.currentText())

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
            data = defsdb.projectile_defs._data_list[new.row()]

            self.cbAnimation.blockSignals(True)
            self.cbAnimationClip.blockSignals(True)

            self.fieldMapper.setCurrentIndex(new.row())
            self.cbAnimation.setCurrentText(data.get('animation', ""))

            # Change anim and clip
            anim_index = self.cbAnimation.currentIndex()
            if anim_index >= 0:
                model = defsdb.animations.get_clip_list_model(anim_index)
                self.cbAnimationClip.setModel(model)
                self.cbAnimationClip.setCurrentText(data.get('animation_key', ""))
            else:
                self.cbAnimationClip.setModel(None)

            self.cbAnimation.blockSignals(False)
            self.cbAnimationClip.blockSignals(False)

            self.controlStack.setCurrentIndex(1)
        else:
            self.controlStack.setCurrentIndex(0)

    def ctx_menu(self, p):
        print(self.lvProjectiles.indexAt(p).isValid())