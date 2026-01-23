from PyQt6.QtCore import QModelIndex, Qt, QStringListModel
from PyQt6.QtWidgets import QDialog, QFileDialog
from ui.Projectiledialog import Ui_projectileDialog
import os
from tools.def_editor import defsdb
import copy

class EmitterListModel(QStringListModel):
    def __init__(self, emitters, parent=None):
        super().__init__(parent)
        self.emitters = emitters
        self.setStringList([e.get('name', 'Unnamed') for e in self.emitters])

    def add_emitter(self, emitter):
        row = len(self.emitters)
        self.insertRow(row)
        self.emitters.append(emitter)
        self.setData(self.index(row), emitter['name'])
        return self.index(row)

    def setData(self, index, value, role=Qt.ItemDataRole.EditRole):
        if role == Qt.ItemDataRole.EditRole:
            self.emitters[index.row()]['name'] = value
        return super().setData(index, value, role)

class ProjectileDialog(QDialog, Ui_projectileDialog):
    def __init__(self, parent=None, weapon_set=None):
        super().__init__()
        self.setupUi(self)

        if weapon_set:
            self.weapon_set = copy.deepcopy(weapon_set)
        else:
            self.weapon_set = { "name": "", "emitters": [] }

        self.tbName.setText(self.weapon_set.get('name', ''))
        self.tbName.textChanged.connect(self.name_changed)

        self.emitter_model = EmitterListModel(self.weapon_set.get('emitters', []))
        self.lvEmitters.setModel(self.emitter_model)
        self.lvEmitters.selectionModel().currentChanged.connect(self.emitter_selection_changed)

        self.btnNewEmitter.pressed.connect(self.add_emitter)

        if len(self.weapon_set.get('emitters', [])) > 0:
            self.lvEmitters.setCurrentIndex(self.emitter_model.index(0))
        else:
            self.stackedControls.setCurrentIndex(0)

    def name_changed(self, text):
        self.weapon_set['name'] = text

    def add_emitter(self):
        new_emitter = {
            "name": f"Emitter {len(self.weapon_set.get('emitters', [])) + 1}",
            "projectile": "",
            "spawns_per_step": 1,
            "delay": 100,
            "start_angle": 0.0,
            "step_angle": 0.0,
            "speed": 1.0,
            "lifetime": 1000,
            "offset": [0.0, 0.0]
        }
        if 'emitters' not in self.weapon_set:
            self.weapon_set['emitters'] = []

        idx = self.emitter_model.add_emitter(new_emitter)
        self.lvEmitters.setCurrentIndex(idx)

    def emitter_selection_changed(self, current, previous):
        self.emitter_changed(current.row())

    def emitter_changed(self, index):
        # If valid emitter (index >= 0), switch to page 1, otherwise page 0.
        if index >= 0:
            self.stackedControls.setCurrentIndex(1)
            emitter = self.weapon_set['emitters'][index]
            self.widget.set_emitter(emitter)
        else:
            self.stackedControls.setCurrentIndex(0)