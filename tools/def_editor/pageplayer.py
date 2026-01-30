from PyQt6.QtCore import QModelIndex, Qt
from PyQt6.QtWidgets import QWidget, QDataWidgetMapper

from tools.def_editor import defsdb
from tools.def_editor.projectiledialog import ProjectileDialog
from ui.Player import Ui_pagePlayer

class pagePlayer(QWidget, Ui_pagePlayer):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        # Mapper
        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(defsdb.player_def)

        self.fieldMapper.addMapping(self.sbMoveSpeed, defsdb.player_def.COL_SPEED)
        self.fieldMapper.addMapping(self.cbAnimation, defsdb.player_def.COL_ANIMATION, b'currentText')
        self.fieldMapper.addMapping(self.cbIdleClip, defsdb.player_def.COL_IDLE_CLIP, b'currentText')
        self.fieldMapper.addMapping(self.cbLeftClip, defsdb.player_def.COL_LEFT_CLIP, b'currentText')
        self.fieldMapper.addMapping(self.cbRightClip, defsdb.player_def.COL_RIGHT_CLIP, b'currentText')

        # Animation clips
        self.cbAnimation.setModel(defsdb.animations)
        self.cbAnimation.setCurrentIndex(0)
        self.cbAnimation.currentIndexChanged.connect(self.on_animation_changed)

        # Weapon sets
        self.weapons_model = defsdb.player_def.make_weapons_model()

        self.lvWeaponSets.setModel(self.weapons_model)
        self.lvWeaponSets.doubleClicked.connect(self.edit_weapon_set)

        self.btnAddWeaponSet.clicked.connect(self.add_weapon_set)
        self.btnDeleteWeaponSet.clicked.connect(self.delete_weapon_set)
        self.btnMoveWeaponSetUp.clicked.connect(self.move_weapon_set_up)
        self.btnMoveWeaponSetDown.clicked.connect(self.move_weapon_set_down)

        defsdb.player_def.modelReset.connect(self.model_reset)
        self.fieldMapper.setCurrentIndex(0)

    def model_reset(self):
        self.fieldMapper.setCurrentIndex(0)

    def on_animation_changed(self, index):
        if index < 0 or self.cbAnimation.signalsBlocked():
            return

        # Set clips list
        model = defsdb.animations.get_clip_list_model(index, include_empty=False)
        self.cbIdleClip.setModel(model)

        model_with_empty = defsdb.animations.get_clip_list_model(index, include_empty=True)
        self.cbLeftClip.setModel(model_with_empty)
        self.cbRightClip.setModel(model_with_empty)
        defsdb.player_def.set_animation(self.cbAnimation.currentText())

    def on_idle_clip_changed(self, index):
        if index < 0 or self.cbIdleClip.signalsBlocked():
            return
        defsdb.player_def.set_idle_clip(self.cbIdleClip.currentText())

    def on_left_clip_changed(self, index):
        if index < 0 or self.cbLeftClip.signalsBlocked():
            return
        defsdb.player_def.set_left_clip(self.cbLeftClip.currentText())

    def on_right_clip_changed(self, index):
        if index < 0 or self.cbRightClip.signalsBlocked():
            return
        defsdb.player_def.set_right_clip(self.cbRightClip.currentText())

    def edit_weapon_set(self, index):
        weapon_set = self.weapons_model.data(index, role=Qt.ItemDataRole.EditRole)
        dlg = ProjectileDialog(self, weapon_set=weapon_set)
        if dlg.exec():
            self.weapons_model.setData(index, dlg.weapon_set)

    def add_weapon_set(self):
        dlg = ProjectileDialog(self, weapon_set=None)
        res = dlg.exec()
        if res:
            self.weapons_model.add(dlg.weapon_set)

    def delete_weapon_set(self):
        idx = self.lvWeaponSets.currentIndex()
        if idx.isValid():
            self.weapons_model.remove(idx.row())
            self.lvWeaponSets.setCurrentIndex(QModelIndex())

    def move_weapon_set_up(self):
        idx = self.lvWeaponSets.currentIndex()
        if idx.isValid():
            row = idx.row()
            self.weapons_model.shift_up(row)
            if row > 0:
                self.lvWeaponSets.setCurrentIndex(self.weapons_model.index(row - 1, 0))

    def move_weapon_set_down(self):
        idx = self.lvWeaponSets.currentIndex()
        if idx.isValid():
            row = idx.row()
            self.weapons_model.shift_down(row)
            if row < self.weapons_model.rowCount() - 1:
                self.lvWeaponSets.setCurrentIndex(self.weapons_model.index(row + 1, 0))