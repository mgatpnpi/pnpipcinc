import fcntl
import os
import sys
import time

ID = sys.argv[1] 


IOCTL_CMD_REG_FREQUENCY = 30720
IOCTL_CMD_CLEAR_AND_START_COUNTER = 30721
IOCTL_CMD_LEAD_COUNTER_AND_TOF = 30722
IOCTL_CMD_INVERSE_SIGNAL_COUNTERS = 30723
IOCTL_CMD_ALLOW_COUNTERS = 30724



FREQUENCY_1MHZ, FREQUENCY_500KHZ, FREQUENCY_250KHZ, FREQUENCY_125KHZ, FREQUENCY_100KHZ, FREQUENCY_50KHZ, FREQUENCY_25KHZ, FREQUENCY_12500HZ = range(8)

f0 = os.open('/dev/pnpipcinc{}counter0'.format(ID), os.O_RDWR)
f1 = os.open('/dev/pnpipcinc{}counter1'.format(ID), os.O_RDWR)

fcntl.ioctl(f0, IOCTL_CMD_LEAD_COUNTER_AND_TOF, 0)
fcntl.ioctl(f0, IOCTL_CMD_INVERSE_SIGNAL_COUNTERS, 0)
fcntl.ioctl(f0, IOCTL_CMD_ALLOW_COUNTERS, 0)
fcntl.ioctl(f0, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_12500HZ+32) // 32 (1<<5) set for the counter that stops another on preset reached
fcntl.ioctl(f1, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_250KHZ)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 4)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 4)
os.pwrite(f0, b'20000', 0)
os.pwrite(f1, b'900000', 0)

fcntl.ioctl(f0, IOCTL_CMD_ALLOW_COUNTERS, 17)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 20)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 20)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 21)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 21)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 23)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 23)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 21)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 21)

for i in range(10):
    time.sleep(1)


    print("12500 Hz ", i+1, " sec ", os.pread(f0, 10, 0))
    print("250 kHz ", i+1, " sec ", os.pread(f1, 10, 0))
os.close(f0)
os.close(f1)

