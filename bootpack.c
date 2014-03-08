#include "bootpack.h"

//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

void HariMain(void)
{
	int i;
	int xsize, ysize;
	init_color();
	char *vram;
	struct BOOTINFO *bootinfo;

	bootinfo = (struct BOOTINFO *)0x0ff0;
	xsize = bootinfo->scrnx;
	ysize = bootinfo->scrny;
	vram = bootinfo->vram;


	// 0xa0000 is start addr of VRAM. We wirte 1 byte(I guess int will be cut into 1 byte or 8 bits) into every addr, then we control all this screen
	//test colpr pallet
	draw_box8(vram, xsize, COL8_FF0000, 0, 0, 30, 30);
	draw_box8(vram, xsize, COL8_0000FF, 20, 10, 40, 40);
	draw_box8(vram, xsize, COL8_008400, 10, 20, 80, 120);

	for (;;)
	{
		io_hlt();
	}
}


void init_color()
{
    static unsigned char color_table[16 * 3]={
    	0x00, 0x00, 0x00,
    	0xff, 0x00, 0x00,
    	0x00, 0xff, 0x00,
    	0xff, 0xff, 0x00,
    	0x00, 0x00, 0xff,
    	0xff, 0x00, 0xff,
    	0x00, 0xff, 0xff,
    	0xff, 0xff, 0xff,
    	0xc6, 0xc6, 0xc6,
    	0x84, 0x00, 0x00,
    	0x00, 0x84, 0x00,
    	0x84, 0x84, 0x00,
    	0x00, 0x00, 0x84,
    	0x84, 0x00, 0x84,
    	0x00, 0x84, 0x84,
    	0x84, 0x84, 0x84
    };
    set_color(0, 15, color_table);
 	return;   
}

void set_color(int start, int end, unsigned char *rgb)
{
	int i, eflags;
	eflags = io_load_eflags();
	io_cli();

	//指向正确的色号
	rgb += (start * 3);

	//0x03c8, port, 指向video DA Converter 地址寄存器
	io_out8(0x03c8, start);
	for (i = start; i < end; ++i)
	{
		io_out8(0x03c9, rgb[0]/4);
		io_out8(0x03c9, rgb[1]/4);
		io_out8(0x03c9, rgb[2]/4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return;
}

void draw_box8(unsigned char *vram, int xsize, unsigned char c, int xs, int ys, int xe, int ye)
{
	int x, y;
    for(y = ys; y <= ye; y++)
    {
    	for (x = xs; x <= xe;x++)
    	{
    		vram[x + y * xsize] = c;
    	}
    }
}
