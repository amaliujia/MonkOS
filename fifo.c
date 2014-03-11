#include "bootpack.h"

#define FLAGS_OVERRUN	0x0001

void FIFOBuffer_Init(struct FIFOBuffer *fifoBuffer, int size, unsigned char *buf)
{
	fifoBuffer->buf = buf;
	fifoBuffer->size = size;
	fifoBuffer->start = 0;
	fifoBuffer->end = 0;
	fifoBuffer->flags = 0;
	fifoBuffer->space = size;
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