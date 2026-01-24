from PyQt6.QtWidgets import QDialog, QDataWidgetMapper
from PyQt6.QtCore import Qt
from ui.Clipdialog import Ui_clipDialog
from tools.def_editor import defsdb

class ClipDialog(QDialog, Ui_clipDialog):
    def __init__(self, clips_model, clip_index, frames_model, global_origin, texture, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.global_origin = global_origin

        # Map clip data
        self.fieldMapper = QDataWidgetMapper(self)
        self.fieldMapper.setModel(clips_model)
        self.fieldMapper.setOrientation(Qt.Orientation.Horizontal)
        self.fieldMapper.setSubmitPolicy(QDataWidgetMapper.SubmitPolicy.ManualSubmit)

        self.fieldMapper.addMapping(self.tbName, defsdb.ClipListModel.COL_NAME)
        self.fieldMapper.addMapping(self.sbFPS, defsdb.ClipListModel.COL_FPS)
        self.fieldMapper.addMapping(self.cbLoopMode, defsdb.ClipListModel.COL_LOOPMODE, b"currentIndex")
        self.fieldMapper.addMapping(self.sbOriginX, defsdb.ClipListModel.COL_ORIGIN_X)
        self.fieldMapper.addMapping(self.sbOriginY, defsdb.ClipListModel.COL_ORIGIN_Y)
        self.fieldMapper.setCurrentIndex(clip_index)

        # Frames mapping
        self.frames_model = frames_model
        self.lvFrames.setModel(self.frames_model)
        self.lvFrames.selectionModel().currentChanged.connect(self.selection_changed)

        self.frameFieldMapper = QDataWidgetMapper(self)
        self.frameFieldMapper.setModel(self.frames_model)
        self.frameFieldMapper.setOrientation(Qt.Orientation.Horizontal)
        self.frameFieldMapper.setSubmitPolicy(QDataWidgetMapper.SubmitPolicy.ManualSubmit)

        self.frameFieldMapper.addMapping(self.sbX, defsdb.ClipFramesModel.COL_X)
        self.frameFieldMapper.addMapping(self.sbY, defsdb.ClipFramesModel.COL_Y)
        self.frameFieldMapper.addMapping(self.sbW, defsdb.ClipFramesModel.COL_W)
        self.frameFieldMapper.addMapping(self.sbH, defsdb.ClipFramesModel.COL_H)
        
        self.frameFieldMapper.setCurrentIndex(0)

        # Frame controls
        self.btnNewFrame.clicked.connect(self.new_frame)
        self.btnDeleteFrame.clicked.connect(self.delete_frame)
        self.btnUpFrame.clicked.connect(self.up_frame)
        self.btnDownFrame.clicked.connect(self.down_frame)

        self.btnSetGlobalOrigin.clicked.connect(self.set_global_origin)

        self.gvTexture.set_texture(texture)
        self.sbX.valueChanged.connect(self.update_editor_rect)
        self.sbY.valueChanged.connect(self.update_editor_rect)
        self.sbW.valueChanged.connect(self.update_editor_rect)
        self.sbH.valueChanged.connect(self.update_editor_rect)

    def selection_changed(self, new, prev):
        if prev.isValid():
            self.frameFieldMapper.submit()
        if new.isValid():
            row = new.row()
            self.frameFieldMapper.setCurrentIndex(row)
            self.update_editor_rect()

    def update_editor_rect(self):
        x = self.sbX.value()
        y = self.sbY.value()
        w = self.sbW.value()
        h = self.sbH.value()
        self.gvTexture.set_selection(x, y, w, h)
        self.frameFieldMapper.submit()

    def new_frame(self):
        self.frames_model.add()

    def delete_frame(self):
        idx = self.lvFrames.currentIndex()
        if idx.isValid():
            self.frames_model.remove(idx.row())

    def up_frame(self):
        idx = self.lvFrames.currentIndex()
        if idx.isValid():
            self.frames_model.shift_up(idx.row())

    def down_frame(self):
        idx = self.lvFrames.currentIndex()
        if idx.isValid():
            self.frames_model.shift_down(idx.row())

    def set_global_origin(self):
        self.sbOriginX.setValue(self.global_origin[0])
        self.sbOriginY.setValue(self.global_origin[1])
