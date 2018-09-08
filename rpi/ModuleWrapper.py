import threading

import serial
import time

import Const
from Modules import ModuleWires, ModuleBigRedButton, ModuleGeneric


class ModuleWrapper:
    def __init__(self, port, bomb):
        self.should_run = False
        self.module = None
        self.module_id = None
        self.lister_thread = None
        self.port_opener_thread = None
        self.bomb = bomb
        self.ser = serial.Serial(
            port=port,
            baudrate=9600,
            parity=serial.PARITY_ODD,
            stopbits=serial.STOPBITS_TWO,
            bytesize=serial.SEVENBITS
        )

    def stop(self):
        self.should_run = False
        time.sleep(1)
        self.__close_port()

    def start(self):
        self.should_run = True
        self.run_port_opener()
        self.run_listener()

    def run_listener(self):
        self.lister_thread = threading.Thread(target=self.__listen)
        self.lister_thread.start()

    def run_port_opener(self):
        self.port_opener_thread = threading.Thread(target=self.__keep_port_open)
        self.port_opener_thread.start()

    def __listen(self):
        while self.should_run:
            if self.ser.isOpen:
                input_data = self.ser.readline()
                if input_data is not None or input_data is not '':
                    action = input_data.split(' ')[0]
                    if action == 'BOOT':
                        m_id = input_data.split(' ')[1]
                        self.module_id = m_id
                        thread = threading.Thread(target=self.__do_boot)
                        thread.start()
                    else:
                        if self.module is None:
                            print "expected BOOT!"
                        else:
                            if action not in Const.VALID_MODULE_ACTIONS:
                                continue
                            thread = threading.Thread(target=self.module.requested_action[action])
                            thread.start()

    def __keep_port_open(self):
        while self.should_run:
            if not self.ser.isOpen():
                try:
                    self.ser.open()
                except Exception as e:
                    print e
                    continue

    def __close_port(self):
        try:
            self.ser.close()
        except Exception as e:
            print e

    def __do_boot(self):
        self.module = self.__get_module()
        self.module.init()
        self.module.boot()

    def __get_module(self):
        if self.module_id == 'wires':
            m = ModuleWires.ModuleWires(self.module_id, self.ser, self.bomb)
            return m
        if self.module_id == 'bigRedButton':
            m = ModuleBigRedButton.ModuleBigRedButton(self.module_id, self.ser, self.bomb)
            return m
        return ModuleGeneric.ModuleGeneric(self.module_id, self.ser, self.bomb)
