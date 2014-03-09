#include "bootpack.h"
#include <stdio.h>

//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

void HariMain(void)
{

	struct BOOTINFO *bootinfo;
	bootinfo = (struct BOOTINFO *)0x0ff0;
	char s[40];
	char cursorBuf[256];
	int mx,my;
	//extern char hankaku[4096];
	
	init_gdtidt();
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

	sprintf(s, "cursor coordinate (%d, %d)", mx, my);
	put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 10, 10);

	for (;;)
	{
		io_hlt();
	}
}
