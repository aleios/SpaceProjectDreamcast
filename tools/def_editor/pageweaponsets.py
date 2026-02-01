from PyQt6.QtCore import Qt, QModelIndex
from PyQt6.QtWidgets import QWidget, QDataWidgetMapper, QInputDialog, QMessageBox

from tools.def_editor import defsdb
from tools.def_editor.models.emitter import EmitterModel
from tools.def_editor.models.weaponset import WeaponSetModel
from tools.def_editor.ui.Weaponsets import Ui_pageWeaponsets

class pageWeaponsets(QWidget, Ui_pageWeaponsets):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.MAX_EMITTERS = 10
        self.setupUi(self)

        self.lvWeaponSets.setModel(defsdb.weaponset_defs)
        self.lvWeaponSets.selectionModel().currentChanged.connect(self.selection_changed)

        self.btnNewWeaponset.pressed.connect(self.new_weaponset)

        self.stackedControls.setCurrentIndex(0)

        # Mapper
        self.mapper = QDataWidgetMapper(self)
        self.mapper.setModel(defsdb.weaponset_defs)
        self.mapper.setOrientation(Qt.Orientation.Horizontal)

        self.mapper.addMapping(self.cbEmitterMode, WeaponSetModel.COL_MODE, b'currentIndex')

        self.mapper.setCurrentIndex(0)

        # Emitters
        self.emitter_model = EmitterModel()
        self.btnNewEmitter.clicked.connect(self.add_emitter)
        self.btnDeleteEmitter.clicked.connect(self.delete_emitter)
        self.btnMoveEmitterUp.clicked.connect(self.move_emitter_up)
        self.btnMoveEmitterDown.clicked.connect(self.move_emitter_down)

        self.emitter_model.rowsInserted.connect(self.row_count_changed)
        self.emitter_model.rowsRemoved.connect(self.row_count_changed)
        self.emitter_model.dataChanged.connect(self._mark_weaponset_modified)

        self.emitterSettings.model.dataChanged.connect(self._mark_weaponset_modified)
        self.lvEmitters.setModel(self.emitter_model)
        self.lvEmitters.selectionModel().currentChanged.connect(self.emitter_changed)

    def select_weaponset(self, name):
        for i in range(defsdb.weaponset_defs.rowCount()):
            idx = defsdb.weaponset_defs.index(i, WeaponSetModel.COL_NAME)
            if defsdb.weaponset_defs.data(idx, Qt.ItemDataRole.DisplayRole) == name:
                self.lvWeaponSets.setCurrentIndex(idx)
                break

    def new_weaponset(self):
        val, res = QInputDialog.getText(self, "Add weapon...", "Name")
        val = val.strip()

        if res:
            if defsdb.weaponset_defs.exists(val):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                defsdb.weaponset_defs.add({
                    "name": val
                })

    def selection_changed(self, new, prev):
        if new.isValid():
            self.mapper.setCurrentIndex(new.row())
            self.emitter_model.set_emitters(defsdb.weaponset_defs.get_emitters(new.row()))

            emitter_idx = self.emitter_model.index(0, 0)
            self.lvEmitters.setCurrentIndex(emitter_idx)
            if not emitter_idx.isValid():
                self.emitterStackedControls.setCurrentIndex(0)

            self.stackedControls.setCurrentIndex(1)
        else:
            self.stackedControls.setCurrentIndex(0)

    def row_count_changed(self, parent, start, end):
        self.btnNewEmitter.setEnabled(self.emitter_model.rowCount() < self.MAX_EMITTERS)
        self._renumber_emitters()
        self._mark_weaponset_modified()

    def _mark_weaponset_modified(self):
        weap_index = self.lvWeaponSets.currentIndex()
        if weap_index.isValid():
            defsdb.weaponset_defs.setData(
                defsdb.weaponset_defs.index(weap_index.row(), WeaponSetModel.COL_MODIFIED),
                True,
                Qt.ItemDataRole.EditRole
            )

    def _renumber_emitters(self):
        weap_index = self.lvWeaponSets.currentIndex()
        if not weap_index.isValid():
            return

        emitters = defsdb.weaponset_defs.get_emitters(weap_index.row())
        for i, emitter in enumerate(emitters):
            new_name = f"Emitter {i + 1}"
            if emitter.get('name') != new_name:
                emitter['name'] = new_name
                # Notify name changed
                idx = self.emitter_model.index(i, EmitterModel.COL_NAME)
                self.emitter_model.dataChanged.emit(idx, idx, [Qt.ItemDataRole.DisplayRole])

    def add_emitter(self):
        if self.emitter_model.rowCount() >= self.MAX_EMITTERS:
            return

        new_emitter = {
            "name": f"Emitter {self.emitter_model.rowCount() + 1}",
            "projectile": defsdb.projectile_defs.data(
                defsdb.projectile_defs.index(0, defsdb.ProjectileModel.COL_NAME), Qt.ItemDataRole.DisplayRole),
            "spawns_per_step": 1,
            "delay": 100,
            "start_angle": 0.0,
            "step_angle": 0.0,
            "speed": 1.0,
            "lifetime": 1000,
            "offset": [0.0, 0.0],
        }

        idx = self.emitter_model.add_emitter(new_emitter)
        self.lvEmitters.setCurrentIndex(idx)

    def delete_emitter(self):
        idx = self.lvEmitters.currentIndex()
        if idx.isValid():
            row = idx.row()
            self.emitter_model.remove(row)

            # Select the next available emitter or clear selection
            new_row = min(row, self.emitter_model.rowCount() - 1)
            if new_row >= 0:
                self.lvEmitters.setCurrentIndex(self.emitter_model.index(new_row, 0))
            else:
                self.lvEmitters.setCurrentIndex(QModelIndex())
                self.emitterStackedControls.setCurrentIndex(0)

    def move_emitter_up(self):
        idx = self.lvEmitters.currentIndex()
        if idx.isValid():
            row = idx.row()
            if self.emitter_model.shift_up(row):
                self.lvEmitters.setCurrentIndex(self.emitter_model.index(row - 1, 0))
                self._renumber_emitters()
                self._mark_weaponset_modified()

    def move_emitter_down(self):
        idx = self.lvEmitters.currentIndex()
        if idx.isValid():
            row = idx.row()
            if self.emitter_model.shift_down(row):
                self.lvEmitters.setCurrentIndex(self.emitter_model.index(row + 1, 0))
                self._renumber_emitters()
                self._mark_weaponset_modified()

    def emitter_changed(self, current, previous):
        if not current.isValid():
            return

        weap_index = self.lvWeaponSets.currentIndex()
        if not weap_index.isValid():
            return

        emitter_index = current.row()

        emitter = defsdb.weaponset_defs.get_emitter(weap_index.row(), emitter_index)
        if emitter is None:
            self.emitterStackedControls.setCurrentIndex(0)
        else:
            self.emitterSettings.set_emitter(emitter)
            self.emitterStackedControls.setCurrentIndex(1)