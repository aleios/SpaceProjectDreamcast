from PyQt6.QtWidgets import QWidget, QInputDialog, QMessageBox, QDataWidgetMapper
from PyQt6.QtGui import QStandardItemModel, QStandardItem
from PyQt6.QtCore import Qt
from ui.Enemies import Ui_pageEnemies
from eventdialog import EventDialog
from tools.def_editor import defsdb
import os

class pageEnemies(QWidget, Ui_pageEnemies):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.lvEnemies.setModel(defsdb.enemy_defs)
        self.lvEnemies.selectionModel().currentChanged.connect(self.selection_changed)

        self.controlStack.setCurrentIndex(0)

        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(defsdb.enemy_defs)
        self.fieldMapper.setOrientation(Qt.Orientation.Horizontal)

        # Map fields to controls
        self.fieldMapper.addMapping(self.tbTexture, defsdb.EnemyModel.COL_TEXTURE)
        self.fieldMapper.addMapping(self.sbHealth, defsdb.EnemyModel.COL_HEALTH)
        self.fieldMapper.addMapping(self.sbColliderRadius, defsdb.EnemyModel.COL_COLLISION_RADIUS)

        # Model animation
        self.cbAnimation.setModel(defsdb.animations)
        self.cbAnimation.setCurrentIndex(0)
        self.cbAnimation.currentIndexChanged.connect(self.on_animation_changed)

        self.cbIdleClip.currentIndexChanged.connect(self.on_idle_clip_changed)
        self.cbLeftClip.currentIndexChanged.connect(self.on_left_clip_changed)
        self.cbRightClip.currentIndexChanged.connect(self.on_right_clip_changed)

        # Map events model
        self.events_model = QStandardItemModel()
        self.lvCmds.setModel(self.events_model)
        self.fieldMapper.currentIndexChanged.connect(self.sync_events)
        self.btnAddCmd.clicked.connect(self.add_cmd)
        self.btnRemoveCmd.clicked.connect(self.remove_cmd)
        self.lvCmds.doubleClicked.connect(self.edit_cmd)
        self.btnUpCmd.clicked.connect(self.move_up_cmd)
        self.btnDownCmd.clicked.connect(self.move_down_cmd)

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
        print(defsdb.game_settings_model.data_obj)

        val, res = QInputDialog.getText(self, "Add enemy...", "Name")

        if res:
            matches = [x for x in defsdb.enemy_defs._data_list if x['name'] == val]
            if matches or os.path.isfile(defsdb.assets_path + "/defs/enemy/" + val + ".json"):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                initial_data = { "name": val, "health": 1, "collision_radius": 1.0 }
                if defsdb.animations.rowCount() > 0:
                    anim_name = defsdb.animations.data(defsdb.animations.index(0, defsdb.AnimationModel.COL_NAME))
                    initial_data["animation"] = anim_name
                    
                    clip_model = defsdb.animations.get_clip_list_model(0)
                    if clip_model.rowCount() > 0:
                        idle_key = clip_model.data(clip_model.index(0, defsdb.ClipListModel.COL_NAME))
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

    def add_cmd(self):
        dlg = EventDialog(self)
        dlg.setData({})
        res = dlg.exec()
        if res:
            cmd = dlg.getData()
            # Find the current enemy and add the command to its events list
            idx = self.fieldMapper.currentIndex()
            if idx >= 0:
                enemy = defsdb.enemy_defs._data_list[idx]
                if 'events' not in enemy or not isinstance(enemy['events'], list):
                    enemy['events'] = []
                enemy['events'].append(cmd)
                enemy['modified'] = True
                self.sync_events(idx)
                
                # Notify changes for the row and specifically for the modified column
                defsdb.enemy_defs.dataChanged.emit(defsdb.enemy_defs.index(idx, 0), defsdb.enemy_defs.index(idx, defsdb.EnemyModel.COL_EVENTS))
                mod_idx = defsdb.enemy_defs.index(idx, defsdb.EnemyModel.COL_MODIFIED)
                defsdb.enemy_defs.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])

    def remove_cmd(self):
        idx = self.lvCmds.currentIndex()
        if not idx.isValid():
            return
        
        row = idx.row()
        enemy_idx = self.fieldMapper.currentIndex()
        if enemy_idx >= 0:
            enemy = defsdb.enemy_defs._data_list[enemy_idx]
            if 'events' in enemy and isinstance(enemy['events'], list):
                if 0 <= row < len(enemy['events']):
                    enemy['events'].pop(row)
                    enemy['modified'] = True
                    self.sync_events(enemy_idx)
                    
                    # Notify changes for the row and specifically for the modified column
                    defsdb.enemy_defs.dataChanged.emit(defsdb.enemy_defs.index(enemy_idx, 0), defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_EVENTS))
                    mod_idx = defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_MODIFIED)
                    defsdb.enemy_defs.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])

    def edit_cmd(self, index):
        if not index.isValid():
            return
        cmd = index.data(Qt.ItemDataRole.UserRole)
        dlg = EventDialog(self)
        dlg.setData(cmd)
        res = dlg.exec()
        if res:
            new_cmd = dlg.getData()
            # Update the command in the enemy list
            row = index.row()
            enemy_idx = self.fieldMapper.currentIndex()
            if enemy_idx >= 0:
                enemy = defsdb.enemy_defs._data_list[enemy_idx]
                if 'events' in enemy and isinstance(enemy['events'], list):
                    if 0 <= row < len(enemy['events']):
                        enemy['events'][row] = new_cmd
                        enemy['modified'] = True
                        self.sync_events(enemy_idx)
                        
                        # Notify changes for the row and specifically for the modified column
                        defsdb.enemy_defs.dataChanged.emit(defsdb.enemy_defs.index(enemy_idx, 0), defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_EVENTS))
                        mod_idx = defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_MODIFIED)
                        defsdb.enemy_defs.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])

    def move_up_cmd(self):
        idx = self.lvCmds.currentIndex()
        if not idx.isValid():
            return
        
        row = idx.row()
        if row <= 0:
            return

        enemy_idx = self.fieldMapper.currentIndex()
        if enemy_idx >= 0:
            enemy = defsdb.enemy_defs._data_list[enemy_idx]
            if 'events' in enemy and isinstance(enemy['events'], list):
                # Swap
                enemy['events'][row], enemy['events'][row-1] = enemy['events'][row-1], enemy['events'][row]
                enemy['modified'] = True
                self.sync_events(enemy_idx)
                
                # Select the moved item
                new_idx = self.events_model.index(row - 1, 0)
                self.lvCmds.setCurrentIndex(new_idx)

                # Notify changes for the row and specifically for the modified column
                defsdb.enemy_defs.dataChanged.emit(defsdb.enemy_defs.index(enemy_idx, 0), defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_EVENTS))
                mod_idx = defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_MODIFIED)
                defsdb.enemy_defs.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])

    def move_down_cmd(self):
        idx = self.lvCmds.currentIndex()
        if not idx.isValid():
            return
        
        row = idx.row()
        enemy_idx = self.fieldMapper.currentIndex()
        if enemy_idx >= 0:
            enemy = defsdb.enemy_defs._data_list[enemy_idx]
            if 'events' in enemy and isinstance(enemy['events'], list):
                if row >= len(enemy['events']) - 1:
                    return

                # Swap
                enemy['events'][row], enemy['events'][row+1] = enemy['events'][row+1], enemy['events'][row]
                enemy['modified'] = True
                self.sync_events(enemy_idx)

                # Select the moved item
                new_idx = self.events_model.index(row + 1, 0)
                self.lvCmds.setCurrentIndex(new_idx)
                
                # Notify changes for the row and specifically for the modified column
                # TODO: just make this a function.
                defsdb.enemy_defs.dataChanged.emit(defsdb.enemy_defs.index(enemy_idx, 0), defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_EVENTS))
                mod_idx = defsdb.enemy_defs.index(enemy_idx, defsdb.EnemyModel.COL_MODIFIED)
                defsdb.enemy_defs.dataChanged.emit(mod_idx, mod_idx, [Qt.ItemDataRole.DisplayRole])

    def sync_events(self, row_index):
        self.events_model.clear()
        events_data = self.fieldMapper.model().index(row_index, defsdb.EnemyModel.COL_EVENTS).data()

        if not isinstance(events_data, list):
            return

        for event in events_data:
            display_name = event.get('type', 'Unknown Event')
            
            item = QStandardItem(display_name)
            item.setData(event, Qt.ItemDataRole.UserRole)
            item.setFlags(Qt.ItemFlag.ItemIsSelectable | Qt.ItemFlag.ItemIsEnabled)
            self.events_model.appendRow(item)