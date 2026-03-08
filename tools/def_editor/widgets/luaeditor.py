from PyQt6.QtGui import QFontMetricsF
from PyQt6.QtWidgets import QPlainTextEdit

from tools.def_editor.widgets.luahighlighter import LuaHighlighter


class LuaEditor(QPlainTextEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.highlighter = LuaHighlighter(self.document())

        space_width = QFontMetricsF(self.font()).horizontalAdvance(' ')
        self.setTabStopDistance(4.0 * space_width)

