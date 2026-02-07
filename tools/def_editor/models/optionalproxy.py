from PyQt6.QtCore import QAbstractProxyModel, QModelIndex, Qt

def _sync_to_model(combo, model, row, column):
    idx = model.index(row, column)
    val = model.data(idx, Qt.ItemDataRole.EditRole)

    blocked = combo.blockSignals(True)
    try:
        if not val:
            combo.setCurrentIndex(0)
        else:
            found = combo.findText(val)
            combo.setCurrentIndex(found if found >= 0 else 0)
    finally:
        combo.blockSignals(blocked)

class OptionalComboProxyModel(QAbstractProxyModel):
    def __init__(self, source_model=None, empty_text="(None)", parent=None):
        super().__init__(parent)
        self._empty_text = empty_text
        self._combo = None
        self._target_model = None
        self._target_column = -1
        self._current_row = -1
        if source_model is not None:
            self.setSourceModel(source_model)

    # Setup similar to how the mapper works.
    def bind(self, combo, target_model, target_column):
        self._combo = combo
        self._target_model = target_model
        self._target_column = target_column
        
        self._combo.currentIndexChanged.connect(self._on_combo_changed)
        self._target_model.dataChanged.connect(self._on_model_changed)

    def set_row(self, row):
        self._current_row = row
        if self._combo and self._target_model and self._target_column != -1:
            _sync_to_model(self._combo, self._target_model, self._current_row, self._target_column)

    def _on_combo_changed(self, index):
        if self._current_row != -1 and not self._combo.signalsBlocked():
            self._update_model(self._combo, self._target_model, self._current_row, self._target_column)

    def _on_model_changed(self, topLeft, bottomRight, roles):
        if self._current_row == -1 or self._combo.signalsBlocked():
            return
        # Check if the model needs to be updated.
        if (topLeft.row() <= self._current_row <= bottomRight.row() and
            topLeft.column() <= self._target_column <= bottomRight.column()):
            if not roles or Qt.ItemDataRole.EditRole in roles or Qt.ItemDataRole.DisplayRole in roles:
                _sync_to_model(self._combo, self._target_model, self._current_row, self._target_column)

    def _update_model(self, combo, model, row, column):
        combo_index = combo.currentIndex()
        value = self.data(self.index(combo_index, 0), Qt.ItemDataRole.EditRole)
        
        model_index = model.index(row, column)
        model.setData(model_index, value)

    def rowCount(self, parent=QModelIndex()):
        if parent.isValid():
            return 0
        sm = self.sourceModel()
        return 1 + (sm.rowCount() if sm else 0)

    def columnCount(self, parent=QModelIndex()):
        if parent.isValid():
            return 0
        sm = self.sourceModel()
        if sm is None:
            return 1
        try:
            return sm.columnCount()
        except (AttributeError, TypeError):
            return 1

    def index(self, row, column, parent=QModelIndex()):
        if parent.isValid():
            return QModelIndex()
        if row < 0 or column < 0 or row >= self.rowCount() or column >= self.columnCount():
            return QModelIndex()
        return self.createIndex(row, column)

    def parent(self, child):
        return QModelIndex()

    def mapToSource(self, proxyIndex: QModelIndex):
        if not proxyIndex.isValid():
            return QModelIndex()

        # Return empty index for the emty row
        if proxyIndex.row() == 0:
            return QModelIndex()
        return self.sourceModel().index(proxyIndex.row() - 1, proxyIndex.column())

    def mapFromSource(self, sourceIndex: QModelIndex):
        if not sourceIndex.isValid():
            return QModelIndex()
        return self.index(sourceIndex.row() + 1, sourceIndex.column())

    def data(self, index, role=Qt.ItemDataRole.DisplayRole):
        if not index.isValid():
            return None

        if index.row() == 0:
            if role == Qt.ItemDataRole.DisplayRole:
                return self._empty_text
            if role == Qt.ItemDataRole.EditRole:
                return ""
            return None

        return self.sourceModel().data(self.mapToSource(index), role)

    def flags(self, index):
        if not index.isValid():
            return Qt.ItemFlag.NoItemFlags

        # Need to allow selecting the none row
        if index.row() == 0:
            return (Qt.ItemFlag.ItemIsEnabled |
                    Qt.ItemFlag.ItemIsSelectable)

        return self.sourceModel().flags(self.mapToSource(index))