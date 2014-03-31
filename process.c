#include "bootpack.h"

struct ProcessCTL *processCTL;
struct Timer *process_timer;

struct Process *Process_init(struct MemoryManager *memoryManager)
{
	int i;
	struct Process *process;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR*)ADR_GDT;
	processCTL = (struct ProcessCTL *)MemoryManagement_alloc_page(memoryManager, sizeof(struct ProcessCTL));
	for (i = 0; i < MAX_TASKS; ++i)
	{
		processCTL->allProcess[i].flags = 0;
		processCTL->allProcess[i].segmentNo = (TASK_GDT0 + i) * 8;
		set_segmdesc(gdt+ TASK_GDT0 + i, 103, (int)&processCTL->allProcess[i].status, AR_TSS32);
	}
	process = Process_alloc();
	process->flags = 2;
	processCTL->online = 1;
	processCTL->runningnow = 0;
	processCTL->processes[0] = process; 
	load_tr(process->segmentNo);
	process_timer = Timer_alloc();
	Timer_SetTimer(process_timer, 2);
	return process;
}

struct Process *Process_alloc()
{
	int i;
	struct Process *process;
	for (i = 0; i < MAX_TASKS; ++i)
	{
		if(processCTL->allProcess[i].flags == 0){
			process = &processCTL->allProcess[i];
			process->flags = 1; 
			process->status.eflags = 0x00000202; 
			process->status.eax = 0; 
			process->status.ecx = 0;
			process->status.edx = 0;
			process->status.ebx = 0;
			process->status.ebp = 0;
			process->status.esi = 0;
			process->status.edi = 0;
			process->status.es = 0;
			process->status.ds = 0;
			process->status.fs = 0;
			process->status.gs = 0;
			process->status.ldtr = 0;
			process->status.iomap = 0x40000000;
			return process;
		}
	}
	return 0;
}

void Process_sleep(struct Process* process)
{
	int i;
	char ts = 0;
	if(process->flags == 2){ // if process is active
		if(process == processCTL->processes[processCTL->runningnow])
		{
			ts = 1;
		}
		for (i = 0; i < processCTL->online; ++i)
		{
			if(processCTL->processes[i] == process)
			{
				break;
			}
		}
		processCTL->online--;
		if(i < processCTL->runningnow)
		{
			processCTL->runningnow--;
		}
		for(; i < processCTL->online; i++)
		{
			processCTL->processes[i] = processCTL->processes[i+1];
		}
		process->flags = 1;
		if (ts != 0)
		{
			if (processCTL->runningnow >= processCTL->online)
			{
				processCTL->runningnow = 0;
			}
			ProcessSwitch(0, processCTL->processes[processCTL->runningnow]->segmentNo);
		}
	}
	return;
}

void Process_run(struct Process *process)
{
	process->flags = 2;
	processCTL->processes[processCTL->online] = process;
	processCTL->online++;
	return;
}

void Process_switch(void)
{
	Timer_SetTimer(process_timer, 2);
	if (processCTL->online >= 2){
		processCTL->runningnow++;
		if(processCTL->runningnow == processCTL->online){
			processCTL->runningnow = 0;
		}
		ProcessSwitch(0, processCTL->processes[processCTL->runningnow]->segmentNo);
	}
	return;
}