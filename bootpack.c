#include "bootpack.h"

//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

void HariMain(void)
{

	struct BOOTINFO *bootinfo;
	bootinfo = (struct BOOTINFO *)0x0ff0;

	extern char hankaku[4096];
	
	init_color();
	init_screen(bootinfo->vram, bootinfo->scrnx, bootinfo->scrny);
/*
Draw Area
*/
	// put_font8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, hankaku+'A'*16, 10, 10);
	// put_font8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, hankaku+'B'*16, 18, 10);
	// put_font8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, hankaku+'C'*16, 26, 10);
	// put_font8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, hankaku+'D'*16, 34, 10);
	put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF,"This is first string", 10, 10);

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
    return;
}


void put_font8(unsigned char *vram, int xsize, unsigned char fontColor, char *c, int x, int y)
{
	int yo;
	int i = 0;
	char d;
	for (yo = y; yo < y+16; ++yo)
	{
		d = c[i];
		if ((d & 0x80) != 0) {vram[yo * xsize + x + 0] = cf;}
		if ((d & 0x40) != 0) {vram[yo * xsize + x + 1] = cf;}
		if ((d & 0x20) != 0) {vram[yo * xsize + x + 2] = cf;}
		if ((d & 0x10) != 0) {vram[yo * xsize + x + 3] = cf;}
		if ((d & 0x08) != 0) {vram[yo * xsize + x + 4] = cf;}
		if ((d & 0x04) != 0) {vram[yo * xsize + x + 5] = cf;}
		if ((d & 0x02) != 0) {vram[yo * xsize + x + 6] = cf;}
		if ((d & 0x01) != 0) {vram[yo * xsize + x + 7] = cf;}
		i++;
	}
	return;
}

void init_screen(char *vram, int xsize, int ysize)
{
	// 0xa0000 is start addr of VRAM. We wirte 1 byte(I guess int will be cut into 1 byte or 8 bits) into every addr, then we control all this screen
	//draw wall paper
	draw_box8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize);
	// draw_box8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	// draw_box8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	// draw_box8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);
	return;
}

void put_string8(unsigned char *vram, int xsize, unsigned char fontColor, char *str, int x, int y)
{
	extern char hankaku[4096];
	char *temp;
	int xt = x;
	for (temp = str; *temp != 0x00; temp++)
	{
		put_font8(vram, xsize, fontColor, hankaku+(*temp)*16, xt, y);
		xt += 8;
	}
	return;
}