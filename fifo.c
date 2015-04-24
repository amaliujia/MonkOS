#include "bootpack.h"

#define FLAGS_OVERRUN	0x0001
//extern struct SHEET *sht_error;

void FIFOBuffer_Init(struct FIFOBuffer *fifoBuffer, int size, unsigned char *buf)
{
    fifoBuffer->buf = buf;
    fifoBuffer->size = size;
    fifoBuffer->start = 0;
    fifoBuffer->end = 0;
    fifoBuffer->flags = 0;
    fifoBuffer->space = size;
    fifoBuffer->process = 0;
}

void FIFOBuffer_Init_Process(struct FIFOBuffer *fifoBuffer, int size, unsigned char *buf, struct Process* process)
{
    fifoBuffer->buf = buf;
    fifoBuffer->size = size;
    fifoBuffer->start = 0;
    fifoBuffer->end = 0;
    fifoBuffer->flags = 0;
    fifoBuffer->space = size;
    fifoBuffer->process = process;
}

int FIFOBuffer_Add(struct FIFOBuffer *fifoBuffer, unsigned char data)
{
    if (fifoBuffer->space == 0)
    {
        fifoBuffer->flags |= FLAGS_OVERRUN;
        return -1;
    }
    fifoBuffer->buf[fifoBuffer->end] = data;
    
    fifoBuffer->end++;
    if (fifoBuffer->end == fifoBuffer->size)
    {
        fifoBuffer->end = 0;
    }
    fifoBuffer->space--;
    if (fifoBuffer->process != 0)
    {
        if (fifoBuffer->process->flags != 2)
        {
            Process_run(fifoBuffer->process);
        }
    }
    return 0;
}

int FIFOBuffer_Get(struct FIFOBuffer *fifoBuffer)
{
    int data;
    if(fifoBuffer->space == fifoBuffer->size)
    {
        
        return -1;
    }
    
    data = fifoBuffer->buf[fifoBuffer->start];
    fifoBuffer->start++;
    
    if (fifoBuffer->start == fifoBuffer->size)
    {
        fifoBuffer->start = 0;
    }
    fifoBuffer->space++;
    return data;
}

int FIFOBuffer_Status(struct FIFOBuffer *fifoBuffer)
{
    return fifoBuffer->size - fifoBuffer->space;
}


/*
	Older verison of IO FIFO
 */
void init_KeyboardBuffer(struct KeyboardBuffer akeyboardBuffer)
{
    akeyboardBuffer.start = 0;
    akeyboardBuffer.end = 1;
    akeyboardBuffer.len = 0;
}

void KeyboardBuffer_Add(char data, struct KeyboardBuffer akeyboardBuffer)
{
    akeyboardBuffer.start = akeyboardBuffer.start % 32;
    akeyboardBuffer.end = akeyboardBuffer.end % 32;
    
    if (akeyboardBuffer.start - akeyboardBuffer.end == 1 || akeyboardBuffer.end - akeyboardBuffer.start == 31)
    {
        return;
    }else{
        akeyboardBuffer.data[akeyboardBuffer.end] = data;
        akeyboardBuffer.end = akeyboardBuffer.end + 1;
        akeyboardBuffer.len++;
        return;
    }
}

char KeyboardBuffer_Remove(struct KeyboardBuffer akeyboardBuffer)
{
    akeyboardBuffer.len--;
    return akeyboardBuffer.data[akeyboardBuffer.start++];
}