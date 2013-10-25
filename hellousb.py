
import usb.core
import time

class hellousb:

    def __init__(self):
        self.HELLO = 0
        self.SET_VALS = 1
        self.GET_VALS = 2
        self.PRINT_VALS = 3
        self.TEST_FORWARD_GO  = 4
        self.TEST_STOP = 5
        self.PRINT_FB = 6
        self.REV = 7
        self.dev = usb.core.find(idVendor = 0x6666, idProduct = 0x0003)
        if self.dev is None:
            raise ValueError('no USB device found matching idVendor = 0x6666 and idProduct = 0x0003')
        self.dev.set_configuration()

    def close(self):
        self.dev = None

    def hello(self):
        try:
            self.dev.ctrl_transfer(0x40, self.HELLO)
        except usb.core.USBError:
            print "Could not send HELLO vendor request."

    def set_vals(self, val1, val2):
        try:
            self.dev.ctrl_transfer(0x40, self.SET_VALS, int(val1), int(val2))
        except usb.core.USBError:
            print "Could not send SET_VALS vendor request."

    def get_vals(self):
        try:
            ret = self.dev.ctrl_transfer(0xC0, self.GET_VALS, 0, 0, 4)
        except usb.core.USBError:
            print "Could not send GET_VALS vendor request."
        else:
            return [int(ret[0])+int(ret[1])*256, int(ret[2])+int(ret[3])*256]

    def print_vals(self):
        try:
            self.dev.ctrl_transfer(0x40, self.PRINT_VALS)
        except usb.core.USBError:
            print "Could not send PRINT_VALS vendor request."
    def test_forward_go(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TEST_FORWARD_GO)
        except usb.core.USBError:
            print "Could not send TEST_FORWARDS vendor request."
    def test_stop(self):
        try:
            self.dev.ctrl_transfer(0x40, self.TEST_STOP)
        except usb.core.USBError:
            print "Could not send TEST_STOP vendor request."
    def print_current(self):
        try:
            self.dev.ctrl_transfer(0x40, self.PRINT_FB)
        except usb.core.USBError:
            print "Could not send PRINT_FB vendor request."
    def reverse_direction(self):
        try:
            self.dev.ctrl_transfer(0x40, self.REV)
        except usb.core.USBError:
            print "Could not send REV vendor request."