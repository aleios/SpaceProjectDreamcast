from PyQt6.QtWidgets import QWidget, QDataWidgetMapper, QMessageBox, QInputDialog
from PyQt6.QtCore import Qt

from tools.def_editor import defsdb
from models import CollectablesModel, ClipListModel, OptionalComboProxyModel
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

        # Map SFX
        self.sfx_model = OptionalComboProxyModel(defsdb.sfx, parent=self.cbSFX)
        self.cbSFX.setModel(self.sfx_model)
        self.sfx_model.bind(self.cbSFX, defsdb.collectable_defs, CollectablesModel.COL_PICKUP_SOUND)

        self.mapper = QDataWidgetMapper(self)
        self.mapper.setModel(defsdb.collectable_defs)

        self.mapper.addMapping(self.cbAnimation, CollectablesModel.COL_ANIM, b'currentText')
        self.mapper.addMapping(self.cbClip, CollectablesModel.COL_ANIM_KEY, b'currentText')
        self.mapper.addMapping(self.sbLifetime, CollectablesModel.COL_LIFETIME)
        self.mapper.addMapping(self.sbColliderRadius, CollectablesModel.COL_COLLIDER_RADIUS)
        self.mapper.addMapping(self.sbSpeed, CollectablesModel.COL_SPEED)
        self.mapper.addMapping(self.sbHealth, CollectablesModel.COL_EFFECT_HEALTH)
        self.mapper.addMapping(self.sbLives, CollectablesModel.COL_EFFECT_LIVES)
        self.mapper.addMapping(self.sbWeaponPower, CollectablesModel.COL_EFFECT_WEAPON)
        self.mapper.addMapping(self.sbScore, CollectablesModel.COL_EFFECT_SCORE)

        self.mapper.setCurrentIndex(0)
        self.sfx_model.set_row(0)

        current_anim = self.cbAnimation.currentIndex()
        self.on_animation_changed(current_anim)

        self.btnNewCollectable.clicked.connect(self.new_collectable)


    def new_collectable(self):
        val, res = QInputDialog.getText(self, "Add collectable...", "Name")
        val = val.strip()

        if res:
            if defsdb.collectable_defs.exists(val):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                initial_data = { "name": val }
                if defsdb.animations.rowCount() > 0:
                    anim_name = defsdb.animations.data(defsdb.animations.index(0, defsdb.AnimationModel.COL_NAME))
                    initial_data["animation"] = anim_name

                    clip_model = defsdb.animations.get_clip_list_model(0)
                    if clip_model.rowCount() > 0:
                        animation_key = clip_model.data(clip_model.index(0, ClipListModel.COL_NAME))
                        initial_data["animation_key"] = animation_key
                defsdb.collectable_defs.add(initial_data)

    def index_changed(self, new, old):
        if new.isValid():
            self.stackedControls.setCurrentIndex(1)
            self.mapper.setCurrentIndex(new.row())
            self.sfx_model.set_row(new.row())
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
