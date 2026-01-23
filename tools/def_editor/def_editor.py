import sys
import os

current_dir = os.path.dirname(os.path.abspath(__file__))
project_root = os.path.abspath(os.path.join(current_dir, "..", ".."))
if project_root not in sys.path:
    sys.path.insert(0, project_root)

from PyQt6 import QtWidgets
from tools.def_editor.mainwindow import MainWindow

app = QtWidgets.QApplication(sys.argv)

main_window = MainWindow()
main_window.show()

app.exec()