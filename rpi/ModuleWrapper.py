import threading

import serial
import time

from Manager import Manager


class ModuleWrapper:
    def __init__(self, port):
        self.should_run = False
        self.module = None
        self.module_id = None
        self.lister_thread = None
        self.port_opener_thread = None
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
        self.lister_thread.run()

    def run_port_opener(self):
        self.port_opener_thread = threading.Thread(target=self.__keep_port_open)
        self.port_opener_thread.run()

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
                        thread.run()
                    else:
                        if self.module is None:
                            print "expected BOOT!"
                        else:
                            thread = threading.Thread(target=self.module.actions[action])
                            thread.run()

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
        self.module = Manager.get_module(self.module_id, self.ser)
        self.module.boot()
