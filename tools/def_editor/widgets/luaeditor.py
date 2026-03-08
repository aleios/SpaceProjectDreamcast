from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFontMetricsF
from PyQt6.QtWidgets import QPlainTextEdit

from tools.def_editor.widgets.luahighlighter import LuaHighlighter


class LuaEditor(QPlainTextEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.highlighter = LuaHighlighter(self.document())

        space_width = QFontMetricsF(self.font()).horizontalAdvance(' ')
        self.setTabStopDistance(4.0 * space_width)

    def wheelEvent(self, e):
        if e.modifiers() & Qt.KeyboardModifier.ControlModifier:
            zoom_dir = e.angleDelta().y()
            font = self.font()
            if zoom_dir > 0:
                font_current = font.pointSize()
                font.setPointSize(font_current + 1)
            elif zoom_dir < 0:
                font_current = font.pointSize()
                font.setPointSize(font_current - 1)

            self.setFont(font)
            space_width = QFontMetricsF(font).horizontalAdvance(' ')
            self.setTabStopDistance(4.0 * space_width)


