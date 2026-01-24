from PyQt6.QtWidgets import QWidget, QInputDialog, QMessageBox, QDataWidgetMapper
from PyQt6.QtCore import Qt
import os

from tools.def_editor.editorsettingsdialog import EditorSettingsDialog
from tools.def_editor.leveleventdialog import LevelEventDialog
from tools.def_editor.levelsettingsdialog import LevelSettingsDialog
from tools.def_editor.models.levels import LevelsModel, LevelEventsModel
from tools.def_editor.ui.Levels import Ui_pageLevelEditor
from tools.def_editor import defsdb

class pageLevelEditor(QWidget, Ui_pageLevelEditor):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.stackedControls.setCurrentIndex(0)

        self.btnNewLevel.pressed.connect(self.add_level)

        self.lvLevels.setModel(defsdb.levels)
        self.lvLevels.clicked.connect(self.level_clicked)

        self.events_model = LevelEventsModel()
        self.lvEvents.setModel(self.events_model)

        self.lvEvents.selectionModel().currentChanged.connect(self.event_selection_changed)
        self.lvEvents.doubleClicked.connect(self.scroll_to_event)
        self.gvLevelEditor.eventSelected.connect(self.gv_event_selected)

        self.gvLevelEditor.itemEdit.connect(self.edit_event)
        self.gvLevelEditor.itemDelete.connect(self.delete_event)

        self.btnEditorSettings.clicked.connect(self.show_editor_settings)
        self.btnLevelSettings.clicked.connect(self.show_level_settings)

    def add_level(self):
        val, res = QInputDialog.getText(self, "Add Level...", "Name")
        if res:
            matches = [x for x in defsdb.levels.levels if x['name'] == val]
            if matches or os.path.isfile(defsdb.assets_path + "/levels/" + val + ".json"):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                defsdb.levels.add(val)

    def level_clicked(self, index):
        level_data = defsdb.levels.levels[index.row()]
        self.events_model.set_events(level_data.get('events', []))
        self.gvLevelEditor.set_level(self.lvLevels.model(), index.row())
        self.stackedControls.setCurrentIndex(1)

    def event_selection_changed(self, current):
        if not current.isValid():
            return
        self.gvLevelEditor.set_active_event_index(current.row())

    def gv_event_selected(self, index):
        self.lvEvents.setCurrentIndex(self.events_model.index(index, 0))

    def edit_event(self, ev, new_ev, idx=-1):
        dialog = LevelEventDialog(ev, new_ev, self)
        res = dialog.exec()
        if res:
            if new_ev:
                self.events_model.add_event(dialog.event)
            else:
                if idx < 0:
                    try:
                        idx = self.events_model.events.index(ev)
                    except ValueError:
                        print("Could not find event to update")
                        return
                
                self.events_model.update_event(idx, dialog.event)

            self.gvLevelEditor.populate_items()

            # Mark level as modified
            level_idx = self.lvLevels.currentIndex()
            if level_idx.isValid():
                mod_idx = defsdb.levels.index(level_idx.row(), LevelsModel.COL_MODIFIED)
                defsdb.levels.setData(mod_idx, True, Qt.ItemDataRole.EditRole)

    def delete_event(self, ev, idx=-1):
        if idx < 0:
            try:
                idx = self.events_model.events.index(ev)
            except ValueError:
                print("Could not find event to delete")
                return

        res = QMessageBox.question(self, "Delete Event", "Are you sure you want to delete this event?",
                                   QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
        if res == QMessageBox.StandardButton.Yes:
            self.events_model.remove_event(idx)
            self.gvLevelEditor.populate_items()

            # Mark level as modified
            level_idx = self.lvLevels.currentIndex()
            if level_idx.isValid():
                mod_idx = defsdb.levels.index(level_idx.row(), LevelsModel.COL_MODIFIED)
                defsdb.levels.setData(mod_idx, True, Qt.ItemDataRole.EditRole)

    def scroll_to_event(self, index):
        if not index.isValid():
            return
        y = self.events_model.events[index.row()]['pos'][1]
        self.gvLevelEditor.scroll_to_time(y)

    def show_editor_settings(self):
        current_settings = {
            'grid_width': self.gvLevelEditor.grid_width,
            'grid_height': self.gvLevelEditor.grid_height,
            'grid_enabled': self.gvLevelEditor.grid_enabled,
            'grid_snap': self.gvLevelEditor.grid_snap
        }
        dlg = EditorSettingsDialog(current_settings, self)
        res = dlg.exec()
        if res:
            settings = {
                'grid_width': dlg.sbGridWidth.value(),
                'grid_height': dlg.sbGridHeight.value(),
                'grid_enabled': dlg.cbShowGrid.isChecked(),
                'grid_snap': dlg.cbSnapGrid.isChecked()
            }
            self.gvLevelEditor.update_settings(settings)

    def show_level_settings(self):
        level_idx = self.lvLevels.currentIndex()
        if level_idx.isValid():
            row = level_idx.row()
            level_data = defsdb.levels.levels[row]
            dlg = LevelSettingsDialog(level_data, self)
            res = dlg.exec()

            if res:
                new_music = dlg.tbInitialMusic.text()
                new_speed = dlg.sbScrollSpeed.value()

                if level_data.get('initial_music') != new_music:
                    mus_idx = defsdb.levels.index(row, LevelsModel.COL_MUSIC)
                    defsdb.levels.setData(mus_idx, new_music, Qt.ItemDataRole.EditRole)

                if level_data.get('scroll_speed') != new_speed:
                    speed_idx = defsdb.levels.index(row, LevelsModel.COL_SPEED)
                    defsdb.levels.setData(speed_idx, new_speed, Qt.ItemDataRole.EditRole)