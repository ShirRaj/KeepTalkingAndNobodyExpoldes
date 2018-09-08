# noinspection PyUnresolvedReferences
#import RPi.GPIO as GPIO

#GPIO.setmode(GPIO.BCM)


class LedIndicator(object):

    def __init__(self, pins):
        """self.pins = pins

        for pin in pins:
            GPIO.setup(pin, GPIO.OUT)
            GPIO.output(pin, GPIO.LOW)

        self.state = 0"""

    def boom(self):
        pass
        #self.state = 8

    def module_solved(self, module_pos):
        pass
        #self.state = self.state | (1 << module_pos)

    def update(self):
        pass
        """
        for i in xrange(4):
            GPIO.output(self.pins[i], (self.state >> i) & 1)"""

    def restart(self):
        pass
        #self.state = 0