#include "bootpack.h"

void HariMain(void)
{
	int i;
	char *p = (char *)0xa0000;
	// 0xa0000 is start addr of VRAM. We wirte 1 byte(I guess int will be cut into 1 byte or 8 bits) into every addr, then we control all this screen
	for (i = 0; i < 0xfa00; ++i)
	{
		//write_mem8(i, 15);
		*(p + i) = 15; 
	}

	for (;;)
	{
		io_hlt();
	}
}