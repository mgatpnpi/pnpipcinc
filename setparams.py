import fcntl
import sys
import time

ID = sys.argv[1] 


IOCTL_CMD_REG_FREQUENCY = 30720
IOCTL_CMD_CLEAR_AND_START_COUNTER = 30721
IOCTL_CMD_LEAD_COUNTER_AND_TOF = 30722
IOCTL_CMD_INVERSE_SIGNAL = 30723
IOCTL_CMD_ALLOW_COUNTER = 30724



FREQUENCY_1MHZ, FREQUENCY_500KHZ, FREQUENCY_250KHZ, FREQUENCY_125KHZ, FREQUENCY_100KHZ, FREQUENCY_50KHZ, FREQUENCY_25KHZ, FREQUENCY_12500HZ = range(8)

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'wb')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'wb')

fcntl.ioctl(f0, IOCTL_CMD_LEAD_COUNTER_AND_TOF, 0)
fcntl.ioctl(f0, IOCTL_CMD_INVERSE_SIGNAL, 0)
fcntl.ioctl(f1, IOCTL_CMD_INVERSE_SIGNAL, 0)
fcntl.ioctl(f0, IOCTL_CMD_ALLOW_COUNTER, 0)
fcntl.ioctl(f1, IOCTL_CMD_ALLOW_COUNTER, 0)
fcntl.ioctl(f0, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_12500HZ+32)
fcntl.ioctl(f1, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_12500HZ+32)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 4)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 4)
f1.close()
f0.close()

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'w')
f0.write('1000123')
f0.close()
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'w')
f1.write('1000003456')
f1.close()

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'wb')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'wb')
fcntl.ioctl(f0, IOCTL_CMD_ALLOW_COUNTER, 1)
fcntl.ioctl(f1, IOCTL_CMD_ALLOW_COUNTER, 1)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 12)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 13)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 15)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 13)
f0.close()
f1.close()

time.sleep(1)


f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'r')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'r')
print("12500 Hz 1 sec ", f0.readline())
print("12500 Hz 1 sec ", f1.readline())
f1.close()
f0.close()

time.sleep(1)


f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'r')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'r')
print("12500 Hz 2 sec ", f0.readline())
print("12500 Hz 2 sec ", f1.readline())
f1.close()
f0.close()

time.sleep(1)


f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'r')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'r')
print("12500 Hz 3 sec ", f0.readline())
print("12500 Hz 3 sec ", f1.readline())
f1.close()
f0.close()
