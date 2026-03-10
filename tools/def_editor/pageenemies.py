from PyQt6.QtCore import Qt
from PyQt6.QtWidgets import QWidget, QInputDialog, QMessageBox, QDataWidgetMapper, QLabel

from tools.def_editor import defsdb
from tools.def_editor.models import ClipListModel, BlankFieldProxyModel
from tools.def_editor.models.enemy import TOTAL_WEAPON_SLOTS
from tools.def_editor.widgets.datacombobox import DataComboBox
from ui.Enemies import Ui_pageEnemies


class pageEnemies(QWidget, Ui_pageEnemies):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.lvEnemies.setModel(defsdb.enemy_defs)
        self.lvEnemies.selectionModel().currentChanged.connect(self.selection_changed)

        self.controlStack.setCurrentIndex(0)

        # Map fields to controls
        self.cbScript.setModel(defsdb.scripts)

        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(defsdb.enemy_defs)
        self.fieldMapper.setOrientation(Qt.Orientation.Horizontal)
        self.fieldMapper.addMapping(self.sbHealth, defsdb.EnemyModel.COL_HEALTH)
        self.fieldMapper.addMapping(self.sbColliderRadius, defsdb.EnemyModel.COL_COLLISION_RADIUS)
        self.fieldMapper.addMapping(self.sbScore, defsdb.EnemyModel.COL_SCORE)
        self.fieldMapper.addMapping(self.cbScript, defsdb.EnemyModel.COL_SCRIPT)


        # Blank field proxy on weaponset_defs
        self.weaponset_mdl = BlankFieldProxyModel()
        self.weaponset_mdl.setSourceModel(defsdb.weaponset_defs)

        # Get model for weapon slots out of EnemyModel
        self.slot_model = defsdb.enemy_defs.get_weaponslot_model(0)

        # Setup slots combos
        self.weap_slot_mapper = QDataWidgetMapper(self)
        self.weap_slot_mapper.setModel(self.slot_model)
        self.weap_slot_mapper.setOrientation(Qt.Orientation.Horizontal)
        for i, x in enumerate(self.slot_model.slots):
            if i >= TOTAL_WEAPON_SLOTS:
                break
            slot_label = QLabel(self)
            slot_label.setText(f"Slot {i}")
            slot_combo = DataComboBox(self)
            slot_combo.setModel(self.weaponset_mdl)
            self.gbWeaponSlots.layout().addWidget(slot_label)
            self.gbWeaponSlots.layout().addWidget(slot_combo)
            self.weap_slot_mapper.addMapping(slot_combo, i, b'data')
        self.weap_slot_mapper.toFirst()

        # Model animation
        self.cbAnimation.setModel(defsdb.animations)
        self.cbAnimation.setCurrentIndex(0)
        self.cbAnimation.currentIndexChanged.connect(self.on_animation_changed)

        self.cbIdleClip.currentIndexChanged.connect(self.on_idle_clip_changed)
        self.cbLeftClip.currentIndexChanged.connect(self.on_left_clip_changed)
        self.cbRightClip.currentIndexChanged.connect(self.on_right_clip_changed)

        # Add enemies
        self.btnAddEnemy.clicked.connect(self.add_enemy)

    def on_animation_changed(self, index):
        if index < 0 or self.cbAnimation.signalsBlocked():
            return

        # Set clips list
        model = defsdb.animations.get_clip_list_model(index, include_empty=False)
        self.cbIdleClip.setModel(model)

        model_with_empty = defsdb.animations.get_clip_list_model(index, include_empty=True)
        self.cbLeftClip.setModel(model_with_empty)
        self.cbRightClip.setModel(model_with_empty)

        # Get enemy index
        enemy_idx = self.lvEnemies.currentIndex()
        if enemy_idx.isValid():
            defsdb.enemy_defs.set_animation(enemy_idx.row(), self.cbAnimation.currentText())

    def on_idle_clip_changed(self, index):
        if index < 0 or self.cbIdleClip.signalsBlocked():
            return

        enemy_idx = self.lvEnemies.currentIndex()
        if enemy_idx.isValid():
            defsdb.enemy_defs.set_idle_key(enemy_idx.row(), self.cbIdleClip.currentText())

    def on_left_clip_changed(self, index):
        if index < 0 or self.cbLeftClip.signalsBlocked():
            return

        enemy_idx = self.lvEnemies.currentIndex()
        if enemy_idx.isValid():
            defsdb.enemy_defs.set_left_key(enemy_idx.row(), self.cbLeftClip.currentText())

    def on_right_clip_changed(self, index):
        if index < 0 or self.cbRightClip.signalsBlocked():
            return

        enemy_idx = self.lvEnemies.currentIndex()
        if enemy_idx.isValid():
            defsdb.enemy_defs.set_right_key(enemy_idx.row(), self.cbRightClip.currentText())

    def add_enemy(self):
        val, res = QInputDialog.getText(self, "Add enemy...", "Name")
        val = val.strip()

        if res:
            if defsdb.enemy_defs.exists(val):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                initial_data = { "name": val, "health": 1, "collision_radius": 1.0 }
                if defsdb.animations.rowCount() > 0:
                    anim_name = defsdb.animations.data(defsdb.animations.index(0, defsdb.AnimationModel.COL_NAME))
                    initial_data["animation"] = anim_name
                    
                    clip_model = defsdb.animations.get_clip_list_model(0)
                    if clip_model.rowCount() > 0:
                        idle_key = clip_model.data(clip_model.index(0, ClipListModel.COL_NAME))
                        initial_data["idle_key"] = idle_key

                defsdb.enemy_defs.add(initial_data)

    def selection_changed(self, new, prev):
        if new.isValid():
            data = defsdb.enemy_defs._data_list[new.row()]

            self.cbAnimation.blockSignals(True)
            self.cbIdleClip.blockSignals(True)
            self.cbLeftClip.blockSignals(True)
            self.cbRightClip.blockSignals(True)

            self.fieldMapper.setCurrentIndex(new.row())
            self.slot_model.set_row(new.row())
            self.cbAnimation.setCurrentText(data.get('animation', ""))

            anim_index = self.cbAnimation.currentIndex()
            if anim_index >= 0:
                model = defsdb.animations.get_clip_list_model(anim_index)
                self.cbIdleClip.setModel(model)

                model_with_empty = defsdb.animations.get_clip_list_model(anim_index, include_empty=True)
                self.cbLeftClip.setModel(model_with_empty)
                self.cbRightClip.setModel(model_with_empty)

                self.cbIdleClip.setCurrentText(data.get('idle_key', ""))
                self.cbLeftClip.setCurrentText(data.get('left_key', ""))
                self.cbRightClip.setCurrentText(data.get('right_key', ""))
            else:
                self.cbIdleClip.setModel(None)
                self.cbLeftClip.setModel(None)
                self.cbRightClip.setModel(None)

            self.cbAnimation.blockSignals(False)
            self.cbIdleClip.blockSignals(False)
            self.cbLeftClip.blockSignals(False)
            self.cbRightClip.blockSignals(False)

            self.controlStack.setCurrentIndex(1)
        else:
            self.controlStack.setCurrentIndex(0)