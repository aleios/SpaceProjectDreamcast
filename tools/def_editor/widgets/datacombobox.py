from PyQt6.QtCore import Qt, pyqtProperty
from PyQt6.QtWidgets import QComboBox

class DataComboBox(QComboBox):
    def getValue(self):
        return self.currentData(Qt.ItemDataRole.UserRole)

    def setValue(self, value):
        for i in range(self.count()):
            if self.itemData(i, Qt.ItemDataRole.UserRole) == value:
                self.setCurrentIndex(i)
                return
        self.setCurrentIndex(0)

    data = pyqtProperty(str, fget=getValue, fset=setValue)