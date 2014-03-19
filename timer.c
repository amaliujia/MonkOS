#include "bootpack.h"


#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040

struct TimerCTL timerCTL;

void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerCTL.count = 0;
	timerCTL.timeout = 0;
	timerCTL.data = 0;
	return;
}


void inthandler20(int *esp)
{
	io_out8(PIC0_OCW2, 0x60); //receive IRQ-00
	timerCTL.count++;
	if (timerCTL.timeout > 0)
	{
		timerCTL.timeout--;
		//process_show();
		if(timerCTL.timeout == 0)
			FIFOBuffer_Add(timerCTL.fifo, timerCTL.data);
	} 
	return;
}

void settimer(unsigned int timeout, struct FIFOBuffer *fifo, unsigned char data)
{
	int eflags;
	eflags = io_load_eflags();
	io_cli();
	timerCTL.timeout = timeout;
	timerCTL.fifo = fifo;
	timerCTL.data = data;
	io_store_eflags(eflags);
	return;
}