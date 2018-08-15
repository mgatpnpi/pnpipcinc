import fcntl
import os
import sys
import time

ID = sys.argv[1] 


IOCTL_CMD_REG_FREQUENCY = ord('x') << 8 | 0
IOCTL_CMD_CLEAR_AND_START_COUNTER = ord('x') << 8 | 1
IOCTL_CMD_LEAD_COUNTER_AND_TOF = ord('x') << 8 | 2
IOCTL_CMD_INVERSE_SIGNAL_COUNTERS = ord('x') << 8 | 3
IOCTL_CMD_ALLOW_COUNTERS = ord('x') << 8 | 4



FREQUENCY_1MHZ, FREQUENCY_500KHZ, FREQUENCY_250KHZ, FREQUENCY_125KHZ, FREQUENCY_100KHZ, FREQUENCY_50KHZ, FREQUENCY_25KHZ, FREQUENCY_12500HZ = range(8)

t0 = os.open('/dev/pnpipcinc{}timer0'.format(ID), os.O_RDWR)
t1 = os.open('/dev/pnpipcinc{}timer1'.format(ID), os.O_RDWR)
c0 = os.open('/dev/pnpipcinc{}counter0'.format(ID), os.O_RDWR)
c1 = os.open('/dev/pnpipcinc{}counter1'.format(ID), os.O_RDWR)

fcntl.ioctl(c0, IOCTL_CMD_LEAD_COUNTER_AND_TOF, 0)

fcntl.ioctl(t0, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_1MHZ+32) # 32 (1<<5)  set for the counter that stops another on preset reached
fcntl.ioctl(t1, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_1MHZ)
fcntl.ioctl(t0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 4)
fcntl.ioctl(t1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 4)
os.pwrite(t0, b'2000000', 0)
os.pwrite(t1, b'2000000', 0)
fcntl.ioctl(c0, IOCTL_CMD_ALLOW_COUNTERS, 0)
fcntl.ioctl(c0, IOCTL_CMD_ALLOW_COUNTERS, 17)
fcntl.ioctl(c0, IOCTL_CMD_LEAD_COUNTER_AND_TOF, 0)

fcntl.ioctl(t0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 20)
fcntl.ioctl(t1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 20)
fcntl.ioctl(t0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 21)
fcntl.ioctl(t1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 21)
fcntl.ioctl(t0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 23)
fcntl.ioctl(t1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 23)

time.sleep(3)


print("timer 0 12500 Hz ",  os.pread(t0, 10, 0))
print("timer 1 250 kHz ",  os.pread(t1, 10, 0))

print("counter 0   ", os.pread(c0, 10, 0))
print("counter 1   ", os.pread(c1, 10, 0))

os.close(t0)
os.close(t1)
fcntl.ioctl(c0, IOCTL_CMD_ALLOW_COUNTERS, 0)
os.close(c0)
os.close(c1)


