#ifndef PNPIPCINC_H
#define PNPIPCINC_H

#define MODULE_NAME "pnpipcinc"

enum {
	FREQUENCY_1MHZ,
	FREQUENCY_500KHZ,
	FREQUENCY_250KHZ,
	FREQUENCY_125KHZ,
	FREQUENCY_100KHZ,
	FREQUENCY_50KHZ,
	FREQUENCY_25KHZ,
	FREQUENCY_12500HZ,
};

#define CS0_ADDR_OFFSET_LEADING_AND_TOF 0
#define CS0_ADDR_OFFSET_INVERTED 1
#define CS0_ADDR_OFFSET_ALLOW 2

#define CS2_ADDR_OFFSET_FREQUENCY_AND_PARAMETERS 9
#define CS2_ADDR_OFFSET_CLEAR_START_COUNTER 10
#define CS2_ADDR_OFFSET_READ_WRITE_PRESET 11
#define CS2_ADDR_OFFSET_READ_COUNTER 15

enum {
	CMD_REG_FREQUENCY,      	// d0..d2 FREQUENCY_...
					// d3 external frequency
					// d4 inverted
					// d5 with preset
					// d6 GATE on the front panel

	CMD_CLEAR_AND_START_COUNTER,	// d0 allow counter
					// d1 start counter (edge from 0 to 1)
					// d2 allow preset
					// d4 allow frequency generation

	CMD_LEAD_COUNTER_AND_TOF,	// d0 TOF(TDC) enable
					// d1 0/1 -> counter0/counter1 is leading 

	CMD_INVERSE_SIGNAL_COUNTERS,	// d0 counter0 inverted signal
					// d4 counter1 inverted signal
					// both files same byte

	CMD_ALLOW_COUNTERS 		// d0 allow counter0
					// d4 allow counter1
					// both files same byte
};

#define IOCTL_CMD_REG_FREQUENCY _IO('x', CMD_REG_FREQUENCY)
#define IOCTL_CMD_CLEAR_AND_START_COUNTER _IO('x', CMD_CLEAR_AND_START_COUNTER)
#define IOCTL_CMD_LEAD_COUNTER_AND_TOF _IO('x', CMD_LEAD_COUNTER_AND_TOF)
#define IOCTL_CMD_INVERSE_SIGNAL_COUNTERS _IO('x', CMD_INVERSE_SIGNAL_COUNTERS)
#define IOCTL_CMD_ALLOW_COUNTERS _IO('x', CMD_ALLOW_COUNTERS)


#define GATE_ON 64
#define TIMER_ON 32
#define INVERTED_ON 16
#define EXTERNAL_FREQUENCY 8

#define CLEAR_COUNT 1
#define START_COUNT 2 
#define CLEAR_REG_COUNT 4
#define ENABLE_FREQUENCY_COUNT 8
#endif
