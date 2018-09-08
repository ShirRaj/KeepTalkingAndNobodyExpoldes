import threading
import sys
import glob

import serial
import zope.event
import random

import Const
from Leds import LedIndicator
import ModuleWrapper


class Manager:
    def __init__(self):
        self.lives = Const.STARTING_LIVES
        self.bomb_type = Const.BOMB_NUM_TO_NAME[random.randint(0, 3)]
        self.module_wrappers = []
        self.modules_order = []
        self.is_started = False
        self.stop_timers = False
        self.solved_counter = 0
        self.sec_counter = 0
        self.init_connection()
        self.leds = LedIndicator([4, 17, 27, 3])
        zope.event.subscribers.append(self.decrease_live)
        zope.event.subscribers.append(self.decrease_time)

    def set_modules_and_order(self):
        comb = random.randint(0, len(Const.ALL_MODULES_OPTIONS)-1)
        self.modules_order = Const.ALL_MODULES_OPTIONS[comb]

    def restart_manager(self):
        self.stop_manager()
        self.__init__()
        self.leds.restart()

    def stop_manager(self):
        for wrapper in self.module_wrappers:
            wrapper.stop()
        # stop main loop
        self.stop_timers = True

    def start_game(self):
        self.is_started = True
        self.run_timer()

    def stop_game(self):
        self.is_started = False
        self.stop_timers = True

    def init_connection(self):
        ports = self.scan_ports()
        self.init_module_wrappers(ports)

    def scan_ports(self):
        if sys.platform.startswith('win'):
            r = ['COM%s' % (i + 1) for i in range(256)]
            l = []
            for i in r:
                try:
                    s = serial.Serial(i)
                    s.close()
                    l.append(i)
                except:
                    continue
            return l

        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            return glob.glob('/dev/tty[A-Za-z]*')

    def init_module_wrappers(self, ports):
        for port in ports:
            module_wrapper = ModuleWrapper.ModuleWrapper(port, self.bomb_type)
            module_wrapper.start()
            self.module_wrappers.append(module_wrapper)

    def update_solved(self, event):
        if not event.startswith(Const.SOLVED_EVENT_KEY):
            return
        self.solved_counter += 1
        module_solved = event.split(':')[1]
        location = self.modules_order.index(Const.MODULES_ID_TO_NUM[module_solved])
        self.leds.module_solved(location)
        m = self.__get_module__(Const.MODULES_NUM_TO_ID[1])
        if self.solved_counter == Const.MODULE_COUNT:
            self.stop_game()
            self.stop_timers = True
            # alert host
        elif m is not None and self.solved_counter == Const.MODULE_COUNT-1:
            m.data()

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
        self.leds.update()
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
        self.leds.boom()
        self.stop_timers = True

    def decrease_live(self, event):
        if event == Const.DECREASE_LIVE_EVENT_KEY:
            self.lives -= 1
            if self.lives < 0:
                self.alert_boom()

    def decrease_time(self, event):
        if event == Const.DECREASE_TIME_EVENT_KEY:
            self.sec_counter += random.randint(20, 40)

    def __get_module__(self, module_id):
        for mw in self.module_wrappers:
            if mw.module.module_id == module_id:
                return mw.module
        return None
