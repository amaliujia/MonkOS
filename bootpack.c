#include "bootpack.h"


//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

//extern struct KeyboardBuffer akeyboardBuffer;
extern struct FIFOBuffer fifoBuffer;
void HariMain(void)
{

	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
	char s[40], cursorBuf[256], keyBuf[32];
	int mx, my;
	int keyboardData;
	
	init_gdtidt();
	init_pic();
	//init_KeyboardBuffer();
	FIFOBuffer_Init(&fifoBuffer, 32, keyBuf);

	//设置CPU的开中断，CPU接收中断
	io_sti();

	init_color();
	init_screen(bootinfo->vram, bootinfo->scrnx, bootinfo->scrny);
/*
Draw Area
*/

// mouse area
	mx = bootinfo->scrnx / 2;
	my = bootinfo->scrny / 2;
	init_mouse_cursor8(cursorBuf, COL8_008484);
	draw_cursor(bootinfo->vram, bootinfo->scrnx, 16, 16 , mx, my, cursorBuf, 16);
	sprintf(s, "(x = %d, y = %d)", mx, my);
	put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);

	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef); 
	for (;;)
	{
		io_cli();
		if (FIFOBuffer_Status(&fifoBuffer) == 0)
		{
			io_stihlt();
		}else{
			keyboardData = FIFOBuffer_Get(&fifoBuffer);
			io_sti();
			sprintf(s, "%02X", keyboardData);
			draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 0,  12*8,16);
			put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
		}
	}
}
