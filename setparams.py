import fcntl
import sys

ID = sys.argv[1] 

IOCTL_CMD_REG_FREQUENCY, IOCTL_CMD_CLEAR_AND_START_COUNTER, IOCTL_CMD_LEAD_COUNTER_AND_TOF, IOCTL_CMD_INVERSE_SIGNAL, IOCTL_CMD_FORBID_ALLOW_CLEAR_COUNTER = range(5)

FREQUENCY_1MHZ, FREQUENCY_500KHZ, FREQUENCY_250KHZ, FREQUENCY_125KHZ, FREQUENCY_100KHZ, FREQUENCY_50KHZ, FREQUENCY_25KHZ, FREQUENCY_12500HZ = range(7)

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'wb')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'wb')

fcntl.ioctl(f0, IOCTL_CMD_LEAD_COUNTER_AND_TOF, 0)
fcntl.ioctl(f0, IOCTL_CMD_INVERSE_SIGNAL, 0)
fcntl.ioctl(f1, IOCTL_CMD_INVERSE_SIGNAL, 0)
fcntl.ioctl(f0, IOCTL_CMD_FORBID_ALLOW_CLEAR_COUNTER, 0)
fcntl.ioctl(f1, IOCTL_CMD_FORBID_ALLOW_CLEAR_COUNTER, 0)
fcntl.ioctl(f0, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_12500HZ+32)
f0.write('1000000000')

fcntl.ioctl(f1, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_12500HZ+32)
f1.write('100000000000')

fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 12)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 12)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 13)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 13)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 15)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 15)

time.sleep(1)

f1.close()
f0.close()

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'r')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'r')
print("12500 Hz 1 sec ", f0.readline())
print("12500 Hz 1 sec ", f1.readline())
f1.close()
f0.close()

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'wb')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'wb')
fcntl.ioctl(f0, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_100KHZ+32)
f0.write('1000000000')

fcntl.ioctl(f1, IOCTL_CMD_REG_FREQUENCY, FREQUENCY_100KHZ+32)
f1.write('100000000000')

fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 12)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 12)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 13)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 13)
fcntl.ioctl(f0, IOCTL_CMD_CLEAR_AND_START_COUNTER, 15)
fcntl.ioctl(f1, IOCTL_CMD_CLEAR_AND_START_COUNTER, 15)

time.sleep(1)

f1.close()
f0.close()

f0 = open('/dev/pnpipcinc{}counter0'.format(ID), 'r')
f1 = open('/dev/pnpipcinc{}counter1'.format(ID), 'r')
print("100 KHz 1 sec ", f0.readline())
print("100 KHz 1 sec ", f1.readline())
f1.close()
f0.close()
