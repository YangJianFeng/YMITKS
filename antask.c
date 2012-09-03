#ifndef DATA_TYPE
	#include "datatype.h"
#endif	

#include <avr/io.h>

void
an_task_portb(void *p_value)
{
	INT8U i;
	INT16U x, y;
	p_value = p_value;
	DDRB = 0xff;
	i = 1;
	for (;;) {
		if (i == 0x00)
			i = 1;
		PORTB = ~i;
		i <<= 1;
		os_time_dly(1);
	}
}
