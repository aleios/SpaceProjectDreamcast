from PyQt6.QtCore import QRegularExpression
from PyQt6.QtGui import QSyntaxHighlighter, QColor, QTextCharFormat


class LuaHighlighter(QSyntaxHighlighter):

    def __init__(self, parent=None):
        super().__init__(parent)

        self.rules = []

        # Keywords
        keyword_format = QTextCharFormat()
        keyword_format.setForeground(QColor(157, 208, 226, 255))

        keywords = [
            "end",
            "function",
            "return",
            "local",

            "if",
            "then",
            "else",
            "elseif"
        ]

        for kwd in keywords:
            self.rules.append({
                'pattern': QRegularExpression('\\b' + kwd + '\\b'),
                'format': keyword_format
            })


        func_format = QTextCharFormat()
        func_format.setForeground(QColor(0, 255, 0, 255))

        self.rules.append({
            'pattern': QRegularExpression('\\b[A-Za-z0-9_]+(?=\\()'),
            'format': func_format
        })

        # Engine provided types
        engine_type_format = QTextCharFormat()
        engine_type_format.setForeground(QColor(255, 0, 0, 255))

        types = [
            "player",
            "enemy",
            "Constants"
        ]

        for kwd in types:
            self.rules.append({
                'pattern': QRegularExpression('\\b' + kwd + '\\b'),
                'format': engine_type_format
            })


    def highlightBlock(self, text):

        for rule in self.rules:
            res = rule['pattern'].globalMatch(text)
            while res.hasNext():
                match = res.next()
                self.setFormat(match.capturedStart(), match.capturedLength(), rule['format'])

        self.setCurrentBlockState(-1)