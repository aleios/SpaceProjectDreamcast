from PyQt6.QtWidgets import QWidget, QDataWidgetMapper, QAbstractItemView, QInputDialog, QMessageBox
from PyQt6.QtCore import Qt
from models.animation import ClipListModel, ClipFramesModel
from ui.Animations import Ui_pageAnimations
from clipdialog import ClipDialog
from tools.def_editor import defsdb
import copy

class pageAnimations(QWidget, Ui_pageAnimations):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.stackedControls.setCurrentIndex(0)

        self.lvAnimations.setModel(defsdb.animations)
        self.lvAnimations.selectionModel().currentChanged.connect(self.selection_changed)

        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(defsdb.animations)
        self.fieldMapper.setOrientation(Qt.Orientation.Horizontal)

        self.fieldMapper.addMapping(self.tbTexture, defsdb.AnimationModel.COL_TEXTURE)
        self.fieldMapper.addMapping(self.sbGlobalOriginX, defsdb.AnimationModel.COL_ORIGIN_X)
        self.fieldMapper.addMapping(self.sbGlobalOriginY, defsdb.AnimationModel.COL_ORIGIN_Y)

        self.lvClips.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)

        self.lvClips.doubleClicked.connect(self.edit_clip)

        self.btnNewClip.clicked.connect(self.add_clip)

        self.btnNewAnimation.clicked.connect(self.add_animation)

    def selection_changed(self, new, prev):
        if new.isValid():
            row = new.row()
            self.fieldMapper.setCurrentIndex(row)

            clip_model = defsdb.animations.get_clip_list_model(row)
            self.lvClips.setModel(clip_model)

            self.stackedControls.setCurrentIndex(1)
        else:
            self.lvClips.setModel(None)
            self.stackedControls.setCurrentIndex(0)

    def edit_clip(self, index):
        if not index.isValid():
            return
        
        clip_index = index.row()
        clips_model_orig = self.lvClips.model()

        # Clone original model
        clips_data = copy.deepcopy(clips_model_orig.clips)

        clips_model_copy = ClipListModel(
            clips_model_orig.parent_model, 
            clips_model_orig.parent_row, 
            data_override=clips_data
        )

        # Grab the global origin for the anim.
        anim_row = self.fieldMapper.currentIndex()
        global_origin = defsdb.animations.get_global_origin(anim_row)

        temp_clip_data = clips_data[clip_index]
        frames_model = ClipFramesModel(temp_clip_data, clips_model_copy)
        dlg = ClipDialog(clips_model_copy, clip_index, frames_model, global_origin, self.tbTexture.text())
        res = dlg.exec()

        if res:
            dlg.fieldMapper.submit()
            dlg.frameFieldMapper.submit()

            row = self.lvAnimations.selectionModel().currentIndex().row()
            defsdb.animations._data_list[row]['clips'] = clips_data
            
            clips_model_orig.beginResetModel()
            clips_model_orig.clips = clips_data
            clips_model_orig.endResetModel()

            clips_model_orig.notify_changed()

    def add_clip(self):
        val, res = QInputDialog.getText(self, "Add clip...", "Name")

        if res:
            clip_model = self.lvClips.model()
            if clip_model:
                if clip_model.exists(val):
                    QMessageBox.critical(self, "Error", "Error: Clip already exists.")
                    return
                clip_model.add(val)

    def add_animation(self):
        val, res = QInputDialog.getText(self, "Add animation...", "Name")
        val = val.strip()

        if res:
            if defsdb.animations.exists(val):
                QMessageBox.critical(self, "Error", "Error: Item already exists.")
            else:
                defsdb.animations.add(val)