from PyQt6.QtWidgets import QDialog, QWidget
from ui.Eventdialog import Ui_eventDialog
import copy

class EventDialog(QDialog, Ui_eventDialog):

    TARGET_MAPPING = {
        'POINT': 1,
        'PLAYER_INITIAL': 0,
        'PLAYER_FOLLOW': 0,
        'DIRECTION': 2,
        'SINE': 3
    }

    def load_move_to(self, cmd):
        self.sbMoveSpeed.setValue(cmd.get("speed", 0.0))

        target = cmd.get("target", "POINT")
        idx = self.cbMoveTarget.findText(target)
        if idx >= 0:
            self.cbMoveTarget.setCurrentIndex(idx)
            self.stackedMoveTarget.setCurrentIndex(self.TARGET_MAPPING.get(target, 0))

        # Load based on target
        if target == "POINT":
            self.sbMoveX.setValue(cmd.get("x", 0.0))
            self.sbMoveY.setValue(cmd.get("y", 0.0))
        if target == "DIRECTION":
            self.sbDirectionAngle.setValue(int(cmd.get("angle", 0)))
            self.sbDirectionAngleStep.setValue(int(cmd.get("angle_step", 0)))
            self.sbDirectionDuration.setValue(int(cmd.get("duration", 0)))
        if target == "SINE":
            self.sbSineAngle.setValue(int(cmd.get("angle", 0)))
            self.sbSinePeriod.setValue(cmd.get("period", 0.0))
            self.sbSineAmplitude.setValue(cmd.get("amplitude", 0.0))
            self.sbSineDuration.setValue(int(cmd.get("duration", 0)))

    def save_move_to(self, cmd):
        cmd["speed"] = self.sbMoveSpeed.value()
        target = self.cbMoveTarget.currentText()
        cmd["target"] = target

        if target == "POINT":
            cmd["x"] = self.sbMoveX.value()
            cmd["y"] = self.sbMoveY.value()
        elif target == "DIRECTION":
            cmd["angle"] = self.sbDirectionAngle.value()
            cmd["angle_step"] = self.sbDirectionAngleStep.value()
            cmd["duration"] = self.sbDirectionDuration.value()
        elif target == "SINE":
            cmd["angle"] = self.sbSineAngle.value()
            cmd["period"] = self.sbSinePeriod.value()
            cmd["amplitude"] = self.sbSineAmplitude.value()
            cmd["duration"] = self.sbSineDuration.value()

    def load_start_firing(self, cmd):
        self.pageStartFiring.set_emitter(copy.deepcopy(cmd))

    def save_start_firing(self, cmd):
        # ensure mapper actually updates the damn value
        self.pageStartFiring.mapper.submit()
        cmd.update(self.pageStartFiring.model.data_dict)

    def load_delay(self, cmd):
        self.sbDelay.setValue(int(cmd.get("duration", 0)))

    def save_delay(self, cmd):
        cmd["duration"] = self.sbDelay.value()

    def load_empty(self, cmd):
        pass

    def save_empty(self, cmd):
        pass

    def save_exit_screen(self, cmd):
        cmd["speed"] = self.sbExitScreenSpeed.value()
    def load_exit_screen(self, cmd):
        self.sbExitScreenSpeed.setValue(cmd.get("speed", 0.0))

    def save_repeat(self, cmd):
        cmd["count"] = self.sbRepeat.value()
        cmd["target"] = self.sbRepeatTarget.value()

    def load_repeat(self, cmd):
        self.sbRepeat.setValue(int(cmd.get("count", 0)))
        self.sbRepeatTarget.setValue(int(cmd.get("target", 0)))

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.setupUi(self)

        self.stackedControls.setCurrentIndex(0)

        self.cbCmd.currentTextChanged.connect(self.page_changed)

        self.pages = {
            "MoveTo": self.pageMoveTo,
            "StopMoving": None,
            "StartFiring": self.pageStartFiring,
            "StopFiring": None,
            "Delay": self.pageDelay,
            "Destroy": None,
            "ExitScreen": self.pageExitScreen,
            "Repeat": self.pageRepeat
        }

        self.emptypage = QWidget()
        self.stackedControls.addWidget(self.emptypage)

        self.cbMoveTarget.currentTextChanged.connect(self.move_target_changed)
        self.stackedMoveTarget.setCurrentIndex(1)

        self.mappers = {
            "MoveTo": {"load": self.load_move_to, "save": self.save_move_to},
            "StopMoving": {"load": self.load_empty, "save": self.save_empty},
            "StartFiring": {"load": self.load_start_firing, "save": self.save_start_firing},
            "StopFiring": {"load": self.load_empty, "save": self.save_empty},
            "Delay": {"load": self.load_delay, "save": self.save_delay},
            "Destroy": {"load": self.load_empty, "save": self.save_empty},
            "ExitScreen": {"load": self.load_exit_screen, "save": self.save_exit_screen},
            "Repeat": {"load": self.load_repeat, "save": self.save_repeat}
        }

    def setData(self, cmd):
        self.cmd = cmd
        cmd_type = cmd.get("type", "")

        if not cmd_type or cmd_type == "":
            return

        idx = self.cbCmd.findText(cmd_type)
        if idx >= 0:
            self.cbCmd.setCurrentIndex(idx)

        mapper = self.mappers.get(cmd_type)
        if mapper:
            loader = mapper.get("load", None)
            if loader:
                loader(self.cmd)

    def getData(self):
        cmd_type = self.cbCmd.currentText()
        res = {"type": cmd_type}

        if hasattr(self, 'cmd'):
            res.update(self.cmd)
            res["type"] = cmd_type

        mapper = self.mappers.get(cmd_type)
        if mapper:
            saver = mapper.get("save", None)
            if saver:
                saver(res)
        return res

    def page_changed(self, text):
        page = self.pages.get(text)
        if page:
            self.stackedControls.setCurrentWidget(page)
        else:
            self.stackedControls.setCurrentWidget(self.emptypage)


    def move_target_changed(self, txt):
        mapping = self.TARGET_MAPPING.get(txt, 0)
        self.stackedMoveTarget.setCurrentIndex(mapping)