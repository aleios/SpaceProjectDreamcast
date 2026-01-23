from PyQt6.QtWidgets import QGraphicsView, QGraphicsScene, QGraphicsRectItem, QGraphicsItem, QMenu
from PyQt6.QtGui import QPen, QBrush, QColor, QTransform
from PyQt6.QtCore import Qt, QTimer, QPoint, QElapsedTimer, pyqtSignal

from tools.def_editor import defsdb
from tools.def_editor.models.levels import LevelsModel


class EventGraphicsItem(QGraphicsRectItem):
    def __init__(self, event, parent):
        super().__init__(-4, -4, 8, 8)
        self.event = event
        self.editor = parent
        self._ignore_changes = False

        self.setFlags(
            QGraphicsItem.GraphicsItemFlag.ItemIsSelectable |
            QGraphicsItem.GraphicsItemFlag.ItemIsMovable |
            QGraphicsItem.GraphicsItemFlag.ItemSendsGeometryChanges
        )
        self.setBrush(QBrush(QColor(255, 0, 0)))
        self.setPen(QPen(Qt.GlobalColor.white, 1))

    def set_active(self, active):
        if active:
            self.setBrush(QBrush(QColor(255, 255, 0))) # yellow = active
        else:
            self.setBrush(QBrush(QColor(255, 0, 0))) # red = inactive

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            # notify editor
            self.editor.item_clicked(self)
        super().mousePressEvent(event)

    def itemChange(self, change, value):
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionChange and self.scene():
            new_pos = value
            if not self._ignore_changes:
                world_x = max(0.0, min(640.0, new_pos.x()))
                world_y = max(0.0, self.editor.viewport_to_world_y(new_pos.y()))

                if self.editor.grid_snap:
                    # Snap to world x and y, where y is essentially 'time'.
                    world_x = round(world_x / self.editor.grid_width) * self.editor.grid_width
                    world_y = round(world_y / self.editor.grid_height) * self.editor.grid_height
                self.event['pos'] = [world_x, world_y]

                if self.editor.model:
                    idx = self.editor.model.index(self.editor.level_row, self.editor.model.COL_MODIFIED)
                    self.editor.model.setData(idx, True, Qt.ItemDataRole.EditRole)

                changed = False
                if abs(world_x - new_pos.x()) > 0.001:
                    new_pos.setX(world_x)
                    changed = True

                # world_y = (480 - screen_y) + current_time
                # screen_y = 480 - (world_time - current_time)
                clamped_screen_y = self.editor.world_to_screen_y(world_y)
                if abs(clamped_screen_y - new_pos.y()) > 0.001:
                    new_pos.setY(clamped_screen_y)
                    changed = True

                if changed:
                    return new_pos

        return super().itemChange(change, value)

class LevelEditor(QGraphicsView):
    eventSelected = pyqtSignal(int)
    itemEdit = pyqtSignal(dict, bool, int)
    itemDelete = pyqtSignal(dict, int)

    def __init__(self, parent=None):
        super().__init__(parent)

        self.model = None
        self.level_row = -1
        self.active_event_index = -1

        self.scroll_preview = False
        self._timer = QTimer(self)
        self._timer.timeout.connect(self._on_tick)
        self._elapsed_timer = QElapsedTimer()
        self._elapsed_timer.start()
        self._timer.start(16)

        self._scene = QGraphicsScene(self)
        self._scene.setSceneRect(0, 0, 640, 480)
        self.setScene(self._scene)

        self.current_pos = 0.0
        self._last_mouse_pos = QPoint()
        self._is_panning = False

        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setBackgroundBrush(Qt.GlobalColor.black)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)

        self.populate_items()

        self.grid_width = 8
        self.grid_height = 8
        self.grid_enabled = True
        self.grid_snap = True

    def _on_tick(self):
        nanos = self._elapsed_timer.nsecsElapsed()
        self._elapsed_timer.restart()

        delta_time = nanos / 1_000_000.0

        if self.scroll_preview:
            idx = defsdb.levels.index(self.level_row, LevelsModel.COL_SPEED)
            scroll_speed = 0.0
            if idx.isValid():
                scroll_speed = defsdb.levels.data(idx, Qt.ItemDataRole.EditRole)#defsdb.game_settings_model.data(idx, Qt.ItemDataRole.EditRole)
                #print("Scroll: ", scroll_speed)
                if scroll_speed is None:
                    scroll_speed = 0.0

            self.current_pos += scroll_speed * delta_time
            self.updateDisplay()

    def set_level(self, model, row):
        self.model = model
        self.level_row = row
        self.active_event_index = -1
        self.populate_items()

    def set_active_event_index(self, index):
        self.active_event_index = index
        self.update_selection_colors()

    def item_clicked(self, item):
        if not self.model or self.level_row < 0:
            return

        events = self.model.data(self.model.index(self.level_row, LevelsModel.COL_EVENTS), Qt.ItemDataRole.EditRole)
        if not events:
            return

        try:
            index = events.index(item.event)
            self.eventSelected.emit(index)
        except ValueError:
            pass

    def update_selection_colors(self):
        items = [item for item in self._scene.items() if isinstance(item, EventGraphicsItem)]
        if not self.model or self.level_row < 0:
            return
            
        events = self.model.data(self.model.index(self.level_row, LevelsModel.COL_EVENTS), Qt.ItemDataRole.EditRole)
        if not events:
            return
            
        target_event = None
        if 0 <= self.active_event_index < len(events):
            target_event = events[self.active_event_index]
            
        for item in items:
            item.set_active(item.event is target_event)

    def viewport_to_world_y(self, screen_y):
        return (480 - screen_y) + self.current_pos

    def world_to_screen_y(self, world_time):
        return 480 - (world_time - self.current_pos)

    def populate_items(self):
        self._scene.clear()

        if not self.model or self.level_row < 0:
            return

        events = self.model.data(self.model.index(self.level_row, LevelsModel.COL_EVENTS), Qt.ItemDataRole.EditRole)

        for i, event in enumerate(events):
            pos = event.get('pos', [320.0, 0.0])
            ex, etime = pos[0], pos[1]

            rect_item = EventGraphicsItem(event, self)
            if i == self.active_event_index:
                rect_item.set_active(True)
                
            screen_y = self.world_to_screen_y(etime)
            rect_item.setPos(ex, screen_y)
            self._scene.addItem(rect_item)

    def update_items(self):
        for item in self._scene.items():
            if isinstance(item, EventGraphicsItem):
                etime = item.event['pos'][1]
                new_y = self.world_to_screen_y(etime)

                item._ignore_changes = True
                item.setPos(item.event['pos'][0], new_y)
                item._ignore_changes = False

    def updateDisplay(self):
        self.update_items()
        self.viewport().update()

    def resizeEvent(self, event):
        self.fitInView(0, 0, 640, 480, Qt.AspectRatioMode.KeepAspectRatio)
        super().resizeEvent(event)

    def wheelEvent(self, event):
        # Scroll "up" increases time
        delta = event.angleDelta().y()
        self.current_pos = max(0.0, self.current_pos + delta)
        self.updateDisplay()

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.MiddleButton:
            self._is_panning = True
            self._last_mouse_pos = event.pos()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        if self._is_panning:
            delta_y = -(event.pos().y() - self._last_mouse_pos.y())
            self.current_pos = max(0.0, self.current_pos - delta_y)
            self._last_mouse_pos = event.pos()
            self.updateDisplay()
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.MiddleButton:
            self._is_panning = False
        super().mouseReleaseEvent(event)

    def mouseDoubleClickEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            ev_pos = self.mapToScene(event.pos())
            item = self._scene.itemAt(ev_pos, QTransform())

            item_idx = -1
            if item:
                # Edit event
                print(f"Double clicked on item: {item.event}")
                if self.model and self.level_row >= 0:
                    events = self.model.data(self.model.index(self.level_row, LevelsModel.COL_EVENTS), Qt.ItemDataRole.EditRole)
                    if events:
                        try:
                            item_idx = events.index(item.event)
                        except ValueError:
                            pass
            else:
                print("No item at mouse position")

            self.itemEdit.emit(item.event if item else {
                "pos": [ev_pos.x(), self.viewport_to_world_y(ev_pos.y())]
            }, item is None, item_idx)


            scene_pos = self.mapToScene(event.pos())
            world_pos = self.current_pos + (480 - scene_pos.y())
            print(f"World pos: {world_pos}")

    def contextMenuEvent(self, event):
        ev_pos = self.mapToScene(event.pos())
        item = self._scene.itemAt(ev_pos, QTransform())

        if item and isinstance(item, EventGraphicsItem):
            menu = QMenu(self)
            edit_action = menu.addAction("Edit")
            delete_action = menu.addAction("Delete")

            action = menu.exec(event.globalPos())

            item_idx = -1
            if self.model and self.level_row >= 0:
                events = self.model.data(self.model.index(self.level_row, LevelsModel.COL_EVENTS), Qt.ItemDataRole.EditRole)
                if events:
                    try:
                        item_idx = events.index(item.event)
                    except ValueError:
                        pass

            if action == edit_action:
                self.itemEdit.emit(item.event, False, item_idx)
            elif action == delete_action:
                self.itemDelete.emit(item.event, item_idx)

    def keyPressEvent(self, event):
        if event.modifiers() & Qt.KeyboardModifier.ShiftModifier:
            speed_mod = 50.0
        else:
            speed_mod = 10.0
        if event.key() == Qt.Key.Key_W:
            self.current_pos += speed_mod
            self.updateDisplay()
        elif event.key() == Qt.Key.Key_S:
            if self.current_pos >= speed_mod:
                self.current_pos -= speed_mod
                self.updateDisplay()
        elif event.key() == Qt.Key.Key_Space:
            self.current_pos = 0.0
            self.updateDisplay()
            self.scroll_preview = not self.scroll_preview


    def drawBackground(self, painter, rect):
        super().drawBackground(painter, rect)

        if self.grid_enabled:
            grid_pen = QPen(QColor(40, 40, 40))
            grid_pen.setCosmetic(True)
            painter.setPen(grid_pen)

            # Draw vertical lines
            for x in range(0, 641, self.grid_width):
                painter.drawLine(x, 0, x, 480)

            # Draw horizontal lines
            start_y = int(self.current_pos % self.grid_height)
            for y_offset in range(-start_y, 481, self.grid_height):
                screen_y = 480 - y_offset
                painter.drawLine(0, screen_y, 640, screen_y)


        #
        # Draw screen boundaries
        #
        screen_height = 480.0
        screen_pen = QPen(QColor(0, 255, 0, 50))
        screen_pen.setWidth(2)
        painter.setPen(screen_pen)

        # Find the first screen boundary relative to the current pos
        first_screen_idx = int(self.current_pos // screen_height)

        # Draw boundaries for the current and next few screens
        for i in range(first_screen_idx, first_screen_idx + 3):
            boundary_px = i * screen_height
            if boundary_px < 0:
                continue

            y = self.world_to_screen_y(boundary_px)

            painter.drawLine(0, int(y), 640, int(y))

            # Label bondaries
            label = f"Screen Boundary ({int(boundary_px)}px)"
            painter.drawText(640 - 200, int(y) - 5, label)

        # Draw Screen Edges
        edge_pen = QPen(Qt.GlobalColor.red)
        edge_pen.setCosmetic(True)
        edge_pen.setStyle(Qt.PenStyle.DashLine)
        painter.setPen(edge_pen)
        painter.drawLine(0, 0, 0, 480)
        painter.drawLine(640, 0, 640, 480)

    def scroll_to_time(self, time):
        self.current_pos = time
        self.updateDisplay()

    def update_settings(self, settings):
        self.grid_enabled = settings.get('grid_enabled', True)
        self.grid_snap = settings.get('grid_snap', False)
        self.grid_width = settings.get('grid_width', 8)
        self.grid_height = settings.get('grid_height', 8)
        self.updateDisplay()