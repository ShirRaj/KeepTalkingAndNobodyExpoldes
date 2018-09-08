import threading
import sys
import glob
import zope.event
import random

import Const
import IModule
import ModuleWrapper


class Manager:
    def __init__(self):
        self.ports = []
        self.module_wrappers = []
        self.modules_order = []
        self.lives = Const.STARTING_LIVES
        self.is_started = False
        self.solved_counter = 0
        self.sec_counter = 0
        self.stop_timers = False
        self.scan_ports()
        self.init_module_wrappers()
        zope.event.subscribers.append(self.decrease_live)

    def set_modules_and_order(self):
        comb = random.randint(len(Const.ALL_MODULES_OPTIONS))
        self.module_wrappers = Const.ALL_MODULES_OPTIONS[comb]

    def restart_manager(self):
        self.stop_manager()
        self.__init__()

    def stop_manager(self):
        for wrapper in self.module_wrappers:
            wrapper.stop()
        # stop main loop
        self.stop_timers = True

    def start_game(self):
        self.is_started = True
        self.run_timer()
        # start main loop
        pass

    def stop_game(self):
        self.is_started = False
        self.stop_timers = True
        # stop main loop
        pass

    def scan_ports(self):
        if sys.platform.startswith('win'):
            self.ports = ['COM%s' % (i + 1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            self.ports = glob.glob('/dev/tty[A-Za-z]*')

    def init_module_wrappers(self):
        for port in self.ports:
            module_wrapper = ModuleWrapper.ModuleWrapper(port)
            module_wrapper.start()
            self.module_wrappers.append(module_wrapper)

    def update_solved(self):
        self.solved_counter += 1
        if self.solved_counter == 3:
            self.stop_game()
            self.stop_timers = True
            # alert host
        # alert host

    def run_timer(self):
        thread = threading.Thread(target=self.__timer())
        thread.run()

    def __timer(self):
        clock_timer = threading.Timer(1.0, self.send_time_clock)
        module_timer = threading.Timer(10.0, self.send_time_module)
        clock_timer.start()
        module_timer.start()
        while not self.stop_timers:
            continue
        clock_timer.cancel()
        module_timer.cancel()

    def send_time_clock(self):
        self.sec_counter += 1
        time_left = Const.GAME_TIME_SEC - self.sec_counter
        # send to clock
        if self.sec_counter >= Const.GAME_TIME_SEC:
            self.alert_boom()

    def send_time_module(self):
        time_left = Const.GAME_TIME_SEC - self.sec_counter
        for mw in self.module_wrappers:
            mw.module.time(time_left)

    def alert_boom(self):
        for mw in self.module_wrappers:
            mw.module.boom()
        # send to host
            self.stop_timers = True

    def decrease_live(self, event):
        if event == Const.DECREASE_LIVE_EVENT_KEY:
            self.lives -= 1
            if self.lives < 0:
                self.alert_boom()

    @staticmethod
    def get_module(m_id, ser):
        if m_id == '':
            m = IModule.IModule(ser)
            return m
