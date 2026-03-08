from PyQt6.QtCore import QRegularExpression, QResource
from PyQt6.QtGui import QSyntaxHighlighter, QColor, QTextCharFormat


class LuaHighlighter(QSyntaxHighlighter):

    def __init__(self, parent=None):
        super().__init__(parent)

        self.rules = []

        # Primitive types
        primitive_format = QTextCharFormat()
        primitive_format.setForeground(QColor(0, 255, 255, 255))

        primitives = [
            # Bool
            "true",
            "false",

            # number (missing some?)
            "\\d+",
            "\\d+.",
            
            # nil type
            "nil"
        ]

        for prim in primitives:
            self.rules.append({
                'pattern': QRegularExpression("\\b" + prim + "\\b"),
                'format': primitive_format
            })

        # Strings
        str_format = QTextCharFormat()
        str_format.setForeground(QColor(255, 204, 153, 255))
        self.rules.append({
            'pattern': QRegularExpression("\"[^\"]*\""),
            'format': str_format
        })
        self.rules.append({
            'pattern': QRegularExpression("\'[^\']*\'"),
            'format': str_format
        })

        # Reserved keywords
        keyword_format = QTextCharFormat()
        keyword_format.setForeground(QColor(255, 153, 153, 255))

        keywords = [
            "end",
            "function",
            "return",
            "break",

            "for",
            "in",
            "until",
            "repeat",
            "while",
            "do",

            "if",
            "then",
            "else",
            "elseif",

            "or",
            "and",
            "not",

            "goto",

            "global",
            "local"
        ]

        for kwd in keywords:
            self.rules.append({
                'pattern': QRegularExpression('\\b' + kwd + '\\b'),
                'format': keyword_format
            })


        func_format = QTextCharFormat()
        func_format.setForeground(QColor(157, 208, 226, 255))

        self.rules.append({
            'pattern': QRegularExpression('\\b[A-Za-z0-9_]+(?=\\()'),
            'format': func_format
        })

        # Engine provided types
        engine_type_format = QTextCharFormat()
        engine_type_format.setForeground(QColor(204, 153, 255, 255))

        types = [
            "player",
            "enemy",
            "Constants",
            "Direction"
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