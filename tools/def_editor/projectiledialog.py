from PyQt6.QtCore import QModelIndex, Qt, QTimer
from PyQt6.QtWidgets import QDialog, QToolTip
from ui.Projectiledialog import Ui_projectileDialog
from tools.def_editor import defsdb
from tools.def_editor.models.emitter import EmitterModel
import copy

class ProjectileDialog(QDialog, Ui_projectileDialog):
    MAX_EMITTERS = 10
    def __init__(self, parent=None, weapon_set=None):
        super().__init__()
        self.setupUi(self)

        if weapon_set:
            self.weapon_set = copy.deepcopy(weapon_set)
            self.orig_name = weapon_set['name']
        else:
            self.orig_name = ''
            self.weapon_set = { "name": "", "mode": 0, "emitters": [] }

        self.tbName.setText(self.weapon_set.get('name', ''))
        self.tbName.textChanged.connect(self.name_changed)

        self.cbEmitterMode.setCurrentIndex(self.weapon_set.get('mode', 0))
        self.cbEmitterMode.currentIndexChanged.connect(self.mode_changed)

        if 'emitters' not in self.weapon_set:
            self.weapon_set['emitters'] = []

        self.emitter_model = EmitterModel()
        self.emitter_model.set_emitters(self.weapon_set['emitters'])
        self.lvEmitters.setModel(self.emitter_model)
        self.lvEmitters.selectionModel().currentChanged.connect(self.emitter_selection_changed)

        self.btnNewEmitter.pressed.connect(self.add_emitter)
        self.emitter_model.rowsInserted.connect(self.row_count_changed)
        self.emitter_model.rowsRemoved.connect(self.row_count_changed)

        self.btnDeleteEmitter.pressed.connect(self.delete_emitter)
        self.btnMoveEmitterUp.pressed.connect(self.move_emitter_up)
        self.btnMoveEmitterDown.pressed.connect(self.move_emitter_down)

        if len(self.weapon_set.get('emitters', [])) > 0:
            self.lvEmitters.setCurrentIndex(self.emitter_model.index(0, 0))
        else:
            self.stackedControls.setCurrentIndex(0)

    def name_changed(self, text):
        self.weapon_set['name'] = text
        self._clear_error()

    def mode_changed(self, idx):
        self.weapon_set['mode'] = idx

    def row_count_changed(self, parent, start, end):
        self.btnNewEmitter.setEnabled(self.emitter_model.rowCount() < self.MAX_EMITTERS)
        self._renumber_emitters()

    def _renumber_emitters(self):
        for i, emitter in enumerate(self.weapon_set['emitters']):
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

    def move_emitter_up(self):
        idx = self.lvEmitters.currentIndex()
        if idx.isValid():
            row = idx.row()
            if self.emitter_model.shift_up(row):
                self.lvEmitters.setCurrentIndex(self.emitter_model.index(row - 1, 0))
                self._renumber_emitters()

    def move_emitter_down(self):
        idx = self.lvEmitters.currentIndex()
        if idx.isValid():
            row = idx.row()
            if self.emitter_model.shift_down(row):
                self.lvEmitters.setCurrentIndex(self.emitter_model.index(row + 1, 0))
                self._renumber_emitters()

    def emitter_selection_changed(self, current, previous):
        self.emitter_changed(current.row())

    def emitter_changed(self, index):
        # If valid emitter (index >= 0), switch to page 1, otherwise page 0.
        if 0 <= index < len(self.weapon_set.get('emitters', [])):
            self.stackedControls.setCurrentIndex(1)
            emitter = self.weapon_set['emitters'][index]
            self.widget.set_emitter(emitter)
        else:
            self.stackedControls.setCurrentIndex(0)

    def accept(self):
        name = self.tbName.text().strip()
        self.weapon_set['name'] = name
        if name == '':
            self._validation_error("Must pick a valid name")
        elif (self.orig_name is not None and name != self.orig_name) and [x for x in defsdb.game_settings.weapons if x['name'] == name]:
            self._validation_error(f"{name} already exists")
        else:
            self._clear_error()
            super().accept()

    def _validation_error(self, msg):
        self.tbName.setStyleSheet("border: 1px solid red;")
        self.tbName.setFocus()
        self.tbName.selectAll()

        QTimer.singleShot(0, lambda: QToolTip.showText(
            self.tbName.mapToGlobal(self.tbName.rect().bottomLeft()),
            msg,
            self.tbName
        ))
    def _clear_error(self):
        self.tbName.setStyleSheet("")