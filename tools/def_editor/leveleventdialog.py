from PyQt6.QtWidgets import QDialog, QWidget
from ui.LevelEventDialog import Ui_levelEventDialog
import copy
from tools.def_editor import defsdb

class LevelEventDialog(QDialog, Ui_levelEventDialog):

    def map_spawn(self):
        ev_data = self.event['event']
        self.cbSpawnerEnemyDef.setCurrentText(ev_data.get('def', ''))

    def save_spawn(self):
        self.event['event']['def'] = self.cbSpawnerEnemyDef.currentText()

    def map_music(self):
        ev_data = self.event['event']
        self.tbMusicKey.setText(ev_data.get('key', ''))
        self.sbMusicFadeIn.setValue(ev_data.get('fade_in', 0.0))
        self.sbMusicFadeOut.setValue(ev_data.get('fade_out', 0.0))

    def save_music(self):
        ev_data = self.event['event']
        ev_data['key'] = self.tbMusicKey.text()
        ev_data['fade_in'] = self.sbMusicFadeIn.value()
        ev_data['fade_out'] = self.sbMusicFadeOut.value()

    def map_wait_clear(self):
        ev_data = self.event['event']
        self.sbWaitClearTimeout.setValue(ev_data.get('timeout', 0.0))
        pass

    def save_wait_clear(self):
        self.event['event']['timeout'] = self.sbWaitClearTimeout.value()

    def map_delay(self):
        ev_data = self.event['event']
        self.sbDelayDuration.setValue(ev_data.get('duration', 0.0))
        pass

    def save_delay(self):
        self.event['event']['duration'] = self.sbDelayDuration.value()

    def map_starfield_speed(self):
        ev_data = self.event['event']
        self.sbStarfieldSpeed.setValue(ev_data.get('speed', 0.0))
        self.sbStarfieldDuration.setValue(ev_data.get('duration', 0.0))
        self.cbStarfieldBlocks.setChecked(ev_data.get('block', False))
        pass

    def save_starfield_speed(self):
        ev_data = self.event['event']
        ev_data['speed'] = self.sbStarfieldSpeed.value()
        ev_data['duration'] = self.sbStarfieldDuration.value()
        ev_data['block'] = self.cbStarfieldBlocks.isChecked()

    def map_clear(self):
        ev_data = self.event['event']
        self.cbClearProjPlayer.setChecked(ev_data.get('player_projectiles', False))
        self.cbClearProjEnemies.setChecked(ev_data.get('enemy_projectiles', False))
        self.cbClearEnemies.setChecked(ev_data.get('enemies', False))
        self.cbClearCollectables.setChecked(ev_data.get('collectables', False))
        pass

    def save_clear(self):
        ev_data = self.event['event']
        ev_data['player_projectiles'] = self.cbClearProjPlayer.isChecked()
        ev_data['enemy_projectiles'] = self.cbClearProjEnemies.isChecked()
        ev_data['enemies'] = self.cbClearEnemies.isChecked()
        ev_data['collectables'] = self.cbClearCollectables.isChecked()

    def __init__(self, event, new_ev, parent=None):
        super().__init__(parent)
        self.setupUi(self)

        self.event = copy.deepcopy(event)
        if 'event' not in self.event:
            self.event['event'] = {'type': 'spawn'} # Default type

        self.cbEventType.currentIndexChanged.connect(self.event_type_changed)

        self.cbSpawnerEnemyDef.setModel(defsdb.enemy_defs)
        self.cbSpawnerEnemyDef.setCurrentIndex(0)

        if not new_ev:
            self.load_event()
        else:
            self.sbPosX.setValue(self.event.get('pos', [0, 0])[0])
            self.sbPosY.setValue(self.event.get('pos', [0, 0])[1])

        # Swap to the correct event pane.
        # If new event then just go to the first index.
        ev_type = self.event['event']['type']
        idx = self.cbEventType.findText(ev_type)
        if idx >= 0:
            self.cbEventType.setCurrentIndex(idx)
            self.stackedEventControls.setCurrentIndex(idx)
        else:
            self.cbEventType.setCurrentIndex(0)
            self.stackedEventControls.setCurrentIndex(0)

    def event_type_changed(self, idx):
        self.stackedEventControls.setCurrentIndex(idx)

    def accept(self):
        self.save_event()
        print('accepted: ', self.event)
        super().accept()

    mappers = {
        'spawn': map_spawn,
        'music': map_music,
        'wait_clear': map_wait_clear,
        'delay': map_delay,
        'starfield_speed': map_starfield_speed,
        'clear': map_clear
    }

    savers = {
        'spawn': save_spawn,
        'music': save_music,
        'wait_clear': save_wait_clear,
        'delay': save_delay,
        'starfield_speed': save_starfield_speed,
        'clear': save_clear
    }

    def load_event(self):
        self.sbPosX.setValue(self.event['pos'][0])
        self.sbPosY.setValue(self.event['pos'][1])

        ev_type = self.event['event']['type']
        idx = self.cbEventType.findText(ev_type)
        if idx < 0:
            return
        self.cbEventType.setCurrentIndex(idx)
        self.mappers[ev_type](self)

    def save_event(self):
        self.event['pos'][0] = self.sbPosX.value()
        self.event['pos'][1] = self.sbPosY.value()

        ev_type = self.cbEventType.currentText()
        self.event['event'] = {'type': ev_type}
        self.savers[ev_type](self)