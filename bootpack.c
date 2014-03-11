#include "bootpack.h"


//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

//extern struct KeyboardBuffer akeyboardBuffer;
extern struct FIFOBuffer fifoBuffer;
extern struct FIFOBuffer mourseFifoBuffer;
void HariMain(void)
{

	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
	char s[40], cursorBuf[256], keyBuf[32];
	int mx, my;
	int i;
	
	init_gdtidt();
	init_pic();
	FIFOBuffer_Init(&fifoBuffer, 32, keyBuf);
	//设置CPU的开中断，CPU接收中断
	io_sti();
	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef); 
	init_color();
	init_screen(bootinfo->vram, bootinfo->scrnx, bootinfo->scrny);
	

	init_keyboard();
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



	enable_mouse();
	for (;;)
	{
		io_cli();
		if (FIFOBuffer_Status(&fifoBuffer) + FIFOBuffer_Status(&mourseFifoBuffer) == 0)
		{
			io_stihlt();
		}else{
			if(FIFOBuffer_Status(&fifoBuffer) != 0){
			i = FIFOBuffer_Get(&fifoBuffer);
			io_sti();
			sprintf(s, "%02X", i);
			draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 0,  12*8,16);
			put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
			}else if(FIFOBuffer_Status(&mourseFifoBuffer) != 0){
			i = FIFOBuffer_Get(&mourseFifoBuffer);
			io_sti();
			sprintf(s, "%02X", i);
			draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 16,  12*8,16);
			put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 16);
			}
		}
	}
}


#define PORT_KEYDAT				0x0060
#define PORT_KEYSTA				0x0064
#define PORT_KEYCMD				0x0064
#define KEYSTA_SEND_NOTREADY	0x02
#define KEYCMD_WRITE_MODE		0x60
#define KBC_MODE				0x47

void wait_KBC_sendready(void)
{
	//等待键盘控制电路准备完毕
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

void init_keyboard(void)
{
	//初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE		0xd4
#define MOUSECMD_ENABLE			0xf4

void enable_mouse(void)
{
	//激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	//如果激活成功，会有一个ACK(1B 0xfa)发送回来
	return; 
}