from PyQt6.QtWidgets import QGraphicsView, QGraphicsScene
from PyQt6.QtGui import QPixmap, QMouseEvent, QPen
from PyQt6.QtCore import Qt
from tools.def_editor import defsdb
import os

class AnimFrameEditor(QGraphicsView):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.tex_path = None
        self.texture = None

        self._scene = QGraphicsScene(self)
        self.setScene(self._scene)

        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setBackgroundBrush(Qt.GlobalColor.black)

        self.setTransformationAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.setResizeAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)

        self.selection_rect = None
        self.origin_plus = []

    def set_texture(self, tex):
        scene = self._scene
        if not scene:
            print("Error: No scene.")
            return

        scene.clear()

        self.tex_path = os.path.join(defsdb.assets_path, "sprites", f"{tex}.png")
        self.texture = QPixmap(self.tex_path)

        if not self.texture.isNull():
            scene.addPixmap(self.texture)
        else:
            print(f"Failed to load texture: {self.tex_path}")

    def wheelEvent(self, event):
        zoom_in_factor = 1.25
        zoom_out_factor = 1 / zoom_in_factor
        if event.angleDelta().y() > 0:
            zoom_factor = zoom_in_factor
        else:
            zoom_factor = zoom_out_factor

        self.scale(zoom_factor, zoom_factor)

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.MiddleButton:
            self.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
            fake_event = QMouseEvent(
                event.type(), 
                event.position(), 
                Qt.MouseButton.LeftButton, 
                Qt.MouseButton.LeftButton, 
                event.modifiers()
            )
            super().mousePressEvent(fake_event)
        else:
            super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.MiddleButton:
            self.setDragMode(QGraphicsView.DragMode.NoDrag)
        super().mouseReleaseEvent(event)

    def set_selection(self, x, y, w, h, origin_x=None, origin_y=None):

        if self.selection_rect and self.selection_rect in self._scene.items():
            self._scene.removeItem(self.selection_rect)
        
        for item in self.origin_plus:
            if item in self._scene.items():
                self._scene.removeItem(item)
        self.origin_plus.clear()

        pen = QPen(Qt.GlobalColor.red, 2)
        pen.setCosmetic(True)
        
        self.selection_rect = self._scene.addRect(x, y, w, h, pen)

        if origin_x is not None and origin_y is not None:
            plus_pen = QPen(Qt.GlobalColor.blue, 2)
            plus_pen.setCosmetic(True)
            size = 2
            l1 = self._scene.addLine(x + origin_x - size, y + origin_y, x + origin_x + size, y + origin_y, plus_pen)
            l2 = self._scene.addLine(x + origin_x, y + origin_y - size, x + origin_x, y + origin_y + size, plus_pen)
            self.origin_plus = [l1, l2]