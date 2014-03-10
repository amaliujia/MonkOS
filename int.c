#include "bootpack.h"

#define PORT_KEYBOARD 0x0060

struct KeyboardBuffer akeyboardBuffer;

void init_pic(void)

{
	//IMR是8位中断屏蔽寄存器，每一位对应一个口，设置为1即为屏蔽
	io_out8(PIC0_IMR,  0xff  ); /* 禁止所有中断 */
	//PIC0,PIC1各有一个IMR
	io_out8(PIC1_IMR,  0xff  ); /* 禁止所有中断 */

	//设置一组硬件，不太理解。ICW1和ICW4似乎有规定的值，就是得这么设定
	//ICW2自由设置，负责分配中断信号和中断口的对应关系
	//ICW3有关主从PIC的设置
	io_out8(PIC0_ICW1, 0x11  ); /* 边沿触发模式 edge trigger mode*/
	io_out8(PIC0_ICW2, 0x20  ); /* IRQ0-7由INT20-27接收*/
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1由IRQ2连接*/
	io_out8(PIC0_ICW4, 0x01  ); /* 无缓冲区模式 */

	io_out8(PIC1_ICW1, 0x11  ); /* 边沿触发模式 edge trigger mode*/
	io_out8(PIC1_ICW2, 0x28  ); /* IRQ8-15由INT28-2f接收*/
	io_out8(PIC1_ICW3, 2     ); /* PIC1由IRQ2连接*/
	io_out8(PIC1_ICW4, 0x01  ); /* 无缓冲区模式*/

	io_out8(PIC0_IMR,  0xfb  ); /* 11111011 PIC1以外全部禁止*/
	io_out8(PIC1_IMR,  0xff  ); /* 11111111 禁止所有中断*/

	return;
}



void init_KeyboardBuffer(struct KeyboardBuffer *keybuf)
{
	
	keybuf->flag = 0;
	keybuf->data = 0;
}


void inthandler21(int *esp)
{
	unsigned char data;
	//IRQ1中断已收到，向0x0060写入0x61, if it's IRQ3, write 0x63
	io_out8(PIC0_OCW2 ,0x61);
	data = io_in8(PORT_KEYBOARD);
	if(akeyboardBuffer.flag == 0)
	{
		akeyboardBuffer.flag = 1;
		akeyboardBuffer.data = data;
	}
	return;
}	

void inthandler2c(int *esp)
{
	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
	draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 16, 32*8-1, 15);
	put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, "Mouse input gets in", 0, 16);
	for (;;) {
		io_hlt();
	}
}
