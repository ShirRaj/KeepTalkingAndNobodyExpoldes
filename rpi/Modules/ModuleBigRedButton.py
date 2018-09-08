import Actions
from IModule import IModule


class ModuleBigRedButton(IModule):
    def boot(self):
        pass

    def data(self):
        self.send_msg_to_arduino(Actions.DATA, "ALL_SOLVED", "1")
