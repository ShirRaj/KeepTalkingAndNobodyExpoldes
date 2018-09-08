import Actions
import Manager
import zope.event


class IModule(object):
    def __init__(self, ser):
        self.is_ready = None
        self.is_ack = None
        self.module_id = None
        self.is_solved = None
        self.ser = ser
        self.dynamic_data = {}
        self.requested_action = {}
        self.set_actions()

    def set_actions(self):
        self.requested_action['BOOT'] = IModule.boot
        self.requested_action['READY'] = IModule.set_ready
        self.requested_action['UNREADY'] = IModule.set_unready
        self.requested_action['ACK'] = IModule.ack
        self.requested_action['PENALTY'] = IModule.penalty
        self.requested_action['TRIGGER'] = IModule.decrease_live
        self.requested_action['SOLVED'] = IModule.solve
        self.requested_action['PING'] = IModule.ping

    def boot(self):
        self.data()

    def set_ready(self):
        self.is_ready = True

    def set_unready(self):
        self.is_ready = False

    def ack(self):
        self.is_ack = True

    def solve(self):
        self.is_solved = True

    @staticmethod
    def decrease_live():
        zope.event.notify("DECREASE_LIVE")

    @staticmethod
    def ping():
        return "pong"

    def penalty(self):
        raise NotImplemented()

    def data(self):
        raise NotImplemented()

    def start(self):
        self.send_msg_to_arduino(Actions.START)

    def time(self, time):
        self.send_msg_to_arduino(Actions.TIME, val=time)

    def boom(self):
        self.send_msg_to_arduino(Actions.START)

    def send_msg_to_arduino(self, action, key=None, val=None):
        if key is None and val is None:
            msg = action
        elif key is None:
            msg = action + ' ' + val
        else:
            msg = action + ' ' + key + ':' + val
        if self.ser.isOpen():
            self.ser.write(msg)
        else:
            print "error"



