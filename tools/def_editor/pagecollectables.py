from PyQt6.QtWidgets import QWidget, QDataWidgetMapper

from tools.def_editor import defsdb
from models import CollectablesModel
from ui.Collectables import Ui_pageCollectables

class pageCollectables(QWidget, Ui_pageCollectables):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.lvCollectables.setModel(defsdb.collectable_defs)

        self.lvCollectables.selectionModel().currentChanged.connect(self.index_changed)
        self.stackedControls.setCurrentIndex(0)

        self.cbAnimation.setModel(defsdb.animations)
        self.cbAnimation.currentIndexChanged.connect(self.on_animation_changed)

        self.mapper = QDataWidgetMapper(self)
        self.mapper.setModel(defsdb.collectable_defs)

        self.mapper.addMapping(self.cbAnimation, CollectablesModel.COL_ANIM, b'currentText')
        self.mapper.addMapping(self.cbClip, CollectablesModel.COL_ANIM_KEY, b'currentText')
        self.mapper.addMapping(self.sbHealth, CollectablesModel.COL_EFFECT_HEALTH)
        self.mapper.addMapping(self.sbLives, CollectablesModel.COL_EFFECT_LIVES)
        self.mapper.addMapping(self.sbWeaponPower, CollectablesModel.COL_EFFECT_WEAPON)

        self.mapper.setCurrentIndex(0)

        current_anim = self.cbAnimation.currentIndex()
        self.on_animation_changed(current_anim)

    def index_changed(self, new, old):
        if new.isValid():
            self.stackedControls.setCurrentIndex(1)
            self.mapper.setCurrentIndex(new.row())
        else:
            self.stackedControls.setCurrentIndex(0)

    def on_animation_changed(self, index):
        if index < 0 or self.cbAnimation.signalsBlocked():
            return

        # Set clips list
        model = defsdb.animations.get_clip_list_model(index)
        self.cbClip.setModel(model)

        # Get projectile index
        proj_idx = self.lvCollectables.currentIndex()
        if proj_idx.isValid():
            defsdb.projectile_defs.set_animation(proj_idx.row(), self.cbAnimation.currentText())