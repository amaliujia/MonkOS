#include "bootpack.h"

#define TIMER_ALLOC 1
#define TIMER_RUNNING 2

struct TimerCTL timerCTL;
extern struct SHEET *sht_back;
extern struct Timer *process_timer;
void init_pit(void)
{
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	int i;
	timerCTL.count = 0;
	timerCTL.next = 0xffffffff;
	timerCTL.active = 0; 
	for (i = 0; i < MAX_TIMER; ++i)
	{
		timerCTL.timers[i].flag = 0; // all unused
	}
	return;
}

struct Timer* Timer_alloc(void)
{
	int i;
	for(i = 0; i < MAX_TIMER; i++)
	{
		if (timerCTL.timers[i].flag == 0)
		{
			timerCTL.timers[i].flag = TIMER_ALLOC;
			return &timerCTL.timers[i];
		}
	}
	return 0;
}

void Timer_free(struct Timer *timer)
{
	timer->flag = 0;
	return;
}

int Timer_init(struct Timer *timer, struct FIFOBuffer *fifo, unsigned char data)
{
		timer->fifo = fifo;
		timer->data = data;
	return 0;
}

int Timer_SetTimer(struct Timer *timer, unsigned int timeout)
{
		int i, j, flags;
		timer->timeout = timeout + timerCTL.count;
		timer->flag = TIMER_RUNNING;
		flags = io_load_eflags();
		io_cli();
		//order
		for(i = 0; i < timerCTL.active; i++)
		{
			if (timerCTL.timersInActive[i]->timeout >= timer->timeout)
			{
				break;
			}
		}
		//move
		for(j = timerCTL.active; j > i; j--)
		{
			timerCTL.timersInActive[j] = timerCTL.timersInActive[j-1];
		}
		timerCTL.active++;
		timerCTL.timersInActive[i] = timer;
		timerCTL.next = timerCTL.timersInActive[0]->timeout;
		io_store_eflags(flags);
	return 0;
}

void inthandler20(int *esp)
{
	int i, j;
	//struct Timer *timer;
	char ts = 0;
	io_out8(PIC0_OCW2, 0x60); //receive IRQ-00
	timerCTL.count++;
	if (timerCTL.next > timerCTL.count)
	{
		//no ome time out
		goto done;
	}

	for (i = 0; i < timerCTL.active; ++i)
	{
		if (timerCTL.timersInActive[i]->timeout > timerCTL.count)
		{
			break;
		}
		timerCTL.timersInActive[i]->flag = TIMER_ALLOC;
		//process_show();
		FIFOBuffer_Add(timerCTL.timersInActive[i]->fifo, timerCTL.timersInActive[i]->data);
	}

	timerCTL.active -= i;
	for(j = 0; j < i ;j++)
	{
		if (timerCTL.timersInActive[i] == process_timer)
		{
			ts = 1;
			break;
		}
	}

	for(j = 0; j < timerCTL.active; j++)
	{
		timerCTL.timersInActive[j] = timerCTL.timersInActive[j+i];
	}

	if (timerCTL.active > 0)
	{
		timerCTL.next = timerCTL.timersInActive[0]->timeout;
	}else{
		timerCTL.next = 0xffffffff;
	}
	if(ts !=0){
		Process_switch();
	}

done:
	return;
}
