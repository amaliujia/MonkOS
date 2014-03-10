#include "bootpack.h"


//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320



void HariMain(void)
{

	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;;
	char s[40];
	int mx, my;
	char cursorBuf[256];
	unsigned char keyboardData;
	
	init_gdtidt();
	init_pic();
	//init_KeyboardBuffer(&akeyboardBuffer);
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
	extern struct KeyboardBuffer akeyboardBuffer;
	for (;;)
	{
		io_cli();
		if (akeyboardBuffer.flag == 0)
		{
			io_stihlt();
		}else{
			keyboardData = akeyboardBuffer.data;
			akeyboardBuffer.flag = 0;
			io_sti();
			sprintf(s, "%02X", keyboardData);
			draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 0,  12*8,16);
			put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
		}
	}
}
