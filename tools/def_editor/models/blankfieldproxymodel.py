from PyQt6.QtCore import QIdentityProxyModel, Qt, QModelIndex

class BlankFieldProxyModel(QIdentityProxyModel):

    def __init__(self, data_field=Qt.ItemDataRole.DisplayRole, parent=None):
        super().__init__(parent)
        self.data_field = data_field

    def rowCount(self, parent=QModelIndex()):
        return super().rowCount(parent) + 1

    def columnCount(self, parent=QModelIndex()):
        return super().columnCount(parent)

    def mapToSource(self, proxy_index):
        if not proxy_index.isValid() or proxy_index.row() == 0:
            return QModelIndex()
        return self.sourceModel().index(proxy_index.row() - 1, proxy_index.column())

    def mapFromSource(self, source_index):
        if not source_index.isValid():
            return QModelIndex()
        return self.createIndex(source_index.row() + 1, source_index.column())

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid():
            return None

        if index.row() == 0:
            if role == Qt.ItemDataRole.DisplayRole:
                return "(None)"
            if role == Qt.ItemDataRole.EditRole:
                return ""
            if role == Qt.ItemDataRole.UserRole:
                return ""
            return None

        src = self.mapToSource(index)
        if role == Qt.ItemDataRole.UserRole:
            return self.sourceModel().data(src, Qt.ItemDataRole.DisplayRole)
        return self.sourceModel().data(src, role)

    def index(self, row, column, parent=QModelIndex()):
        if parent.isValid():
            return QModelIndex()
        return self.createIndex(row, column)

    def flags(self, index):
        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags

        if index.row() == 0:
            return (
                    Qt.ItemFlag.ItemIsEnabled |
                    Qt.ItemFlag.ItemIsSelectable
            )

        return self.sourceModel().flags(self.mapToSource(index))