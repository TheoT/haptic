from hellousb import *
from Tkinter import *
import time
usb=hellousb()
usb.set_vals(0,0)
usb.print_vals()

usb.test_forward_go()
for i in range(50):
	usb.print_current()
	time.sleep(.3)
	# usb.reverse_direction()
usb.test_stop()
usb.print_vals()