import struct
import time

kernel_file = open("./kernel8.img", "rb")
kernel_raw = kernel_file.read()
kernel_size = len(kernel_raw)

with open('/dev/ttyUSB0', "wb", buffering = 0) as tty:
    sending = struct.pack("<I", kernel_size)
    tty.write(sending)
    tty.flush()

    # time.sleep(1)
    
    # for i in kernel_raw:
    # tty.write(kernel_raw)
    # tty.flush()