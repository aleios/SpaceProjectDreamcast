from PyQt6.QtWidgets import QWidget, QDataWidgetMapper
from PyQt6.QtGui import QGuiApplication
from tools.def_editor.ui.Emittersettings import Ui_emitterSettings
import tools.def_editor.defsdb as defsdb
from tools.def_editor.models.emitter import EmitterModel
import json
import copy

class EmitterSettings(QWidget, Ui_emitterSettings):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setupUi(self)

        self.cbProjectile.setModel(defsdb.projectile_defs)
        self.cbTarget.currentIndexChanged.connect(self.target_updated)

        self.model = EmitterModel()
        self.mapper = QDataWidgetMapper()
        self.mapper.setModel(self.model)

        self.mapper.addMapping(self.cbProjectile, EmitterModel.COL_PROJECTILE, b"currentText")
        self.mapper.addMapping(self.sbSpawnPerStep, EmitterModel.COL_SPAWNS)
        self.mapper.addMapping(self.sbProjectileDelay, EmitterModel.COL_DELAY)
        self.mapper.addMapping(self.sbStartAngle, EmitterModel.COL_START_ANGLE)
        self.mapper.addMapping(self.sbAngleStep, EmitterModel.COL_STEP_ANGLE)
        self.mapper.addMapping(self.sbSpeed, EmitterModel.COL_SPEED)
        self.mapper.addMapping(self.sbLifetime, EmitterModel.COL_LIFETIME)
        self.mapper.addMapping(self.sbOffsetX, EmitterModel.COL_OFFSETX)
        self.mapper.addMapping(self.sbOffsetY, EmitterModel.COL_OFFSETY)

        self.mapper.addMapping(self.cbTarget, EmitterModel.COL_TARGET, b"currentIndex")

        self.mapper.addMapping(self.cbTracking, EmitterModel.COL_TARGET_TRACKING, b"currentIndex")
        self.mapper.addMapping(self.sbTrackingDelay, EmitterModel.COL_TRACKING_DELAY)

        self.btnCopyEmitter.clicked.connect(self.copy_emitter)
        self.btnPasteEmitter.clicked.connect(self.paste_emitter)

        self.mapper.setCurrentIndex(0)

        self.target_updated(self.cbTarget.currentIndex())

        # Tooltips
        self.cbTarget.setToolTip("Target types:\nNone - Do not track target\nNearest - Acquire nearest target\nStrongest - Acquire target with most max HP")

        tracking_tt = "Tracking types:\nSnapshot - Select target at projectile spawn\nAcquire Once - Select target after tracking delay\nContinuous - Follow target until invalid or dead"
        self.cbTracking.setToolTip(tracking_tt)

    def target_updated(self, index):
        if index == 0:
            self.targetStack.setCurrentIndex(0)
        else:
            self.targetStack.setCurrentIndex(1)

    def set_emitter(self, emitter_dict):
        self.model.set_data(emitter_dict)
        self.mapper.toFirst()

    def copy_emitter(self):
        if not self.model.data_dict:
            return
        
        clipboard = QGuiApplication.clipboard()
        clipboard.setText(json.dumps(self.model.export_data()))

    def paste_emitter(self):
        clipboard = QGuiApplication.clipboard()
        try:
            data = json.loads(clipboard.text())
            # Don't overwrite the name
            name = self.model.data_dict.get('name', 'Emitter')
            self.model.data_dict.update(data)
            self.model.data_dict['name'] = name
            self.model.set_data(self.model.data_dict) # Refresh model
            self.mapper.toFirst()
        except:
            pass