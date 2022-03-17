import struct
import time

kernel_file = open("./kernel8.img", "rb")
kernel_raw = kernel_file.read()
kernel_size = len(kernel_raw)

with open('/dev/ttyUSB0', "wb", buffering = 0) as tty:
    print("sending kernel_size...")
    print(hex(kernel_size))
    sending = struct.pack("<I", kernel_size)
    tty.write(sending)
    tty.flush()

    print("sending kernel...")

    time.sleep(1)
    
    for c in kernel_raw:
        tty.write(bytes([c]))
        tty.flush()