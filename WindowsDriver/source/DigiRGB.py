#!/usr/bin/python
import usb # 1.0 not 0.4

import sys
import time
sys.path.append("..")

from arduino.usbdevice import ArduinoUsbDevice

def send_param(theDevice, value):
    isSend = False
    while isSend is False:
        try:
            theDevice.write(value)
            resp = int(theDevice.read())
            print("send ", value, " recv ", resp)
            isSend = True
            pass
        except Exception as e:
            print("failed", e)
            pass
    return

def init_function():
    if len(sys.argv)==1:
        print("Usage [<Mode>] [<param>..]")
        return

    mode = int(sys.argv[1])

    theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
    if theDevice is None:
        print("Cannot found the devie")
        return

    print ("Found: 0x%04x 0x%04x %s %s" % (theDevice.idVendor, 
                                          theDevice.idProduct,
                                          theDevice.productName,
                                          theDevice.manufacturer))
    
    # mode
    send_param(theDevice, mode)
    
    # params
    for x in range(2,len(sys.argv)):
        send_param(theDevice, int(sys.argv[x]))
        pass

    return                  

if __name__ == "__main__":
    init_function()
