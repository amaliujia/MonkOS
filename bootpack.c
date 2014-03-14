#include "bootpack.h"


//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

//extern struct KeyboardBuffer akeyboardBuffer;
extern struct FIFOBuffer fifoBuffer;
extern struct FIFOBuffer mourseFifoBuffer;
void HariMain(void)
{

	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
	char s[40], cursorBuf[256], keyBuf[32], mouBuf[128];
	char mouses[40];
	int mx, my;
	int i = 0;
	unsigned int totalMemory;
//	struct MemoryManager memoryManager;
	int mouseCheckerStatus;
	struct MouseChecker mouseChecker;
	init_MouseChecker(&mouseChecker);
	
	init_gdtidt();
	init_pic();
	FIFOBuffer_Init(&fifoBuffer, 32, keyBuf);
	FIFOBuffer_Init(&mourseFifoBuffer, 128, mouBuf);
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
	
	enable_mouse();

	// 初始化内存管理

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
				draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_008484, 0, 0,  12*8,16);
				put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
			}else if(FIFOBuffer_Status(&mourseFifoBuffer) != 0){
				i = FIFOBuffer_Get(&mourseFifoBuffer);
				io_sti();
				mouseCheckerStatus = MouseChecker_Job(&mouseChecker, i);
				if(mouseCheckerStatus != 0 && mouseCheckerStatus != -1){
					sprintf(s, "[lcr %4d %4d]", mouseChecker.x, mouseChecker.y);
					if ((mouseChecker.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mouseChecker.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mouseChecker.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_008484, 0, 16,  12*8*3, 32);
					put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 16);
					
					draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_008484, mx, my, mx+16, my+16);

					mx += mouseChecker.x;
					my -= mouseChecker.y;

					if(mx < 0)
						mx = 0;
					if(my < -16)
						my = -16;
					if(mx > bootinfo->scrnx-16 )
						mx = bootinfo->scrnx-16 ;
					if(my > bootinfo->scrny + 16)
						my = bootinfo->scrny + 16;
					sprintf(mouses, "x=%d y=%d",mx, my);
					draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_008484, 12*8+10, 0,  12*8+12*8+10,16);
					put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, mouses, 12*8+10, 0);
					draw_cursor(bootinfo->vram, bootinfo->scrnx, 16, 16 , mx, my, cursorBuf, 16);
				}		

			}
		}
	}
}
