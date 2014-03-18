#include "bootpack.h"

//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

extern struct FIFOBuffer fifoBuffer;
extern struct FIFOBuffer mourseFifoBuffer;


void HariMain(void)
{

	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
	char s[40], keyBuf[32], mouBuf[128];
	char memTest[40];
	char mouses[40];
	int mx, my;
	int i = 0;
	unsigned int totalMemory;
	struct MemoryManager *memoryManager = (struct memoryManager *)MEMMAN_ADDR;
	int mouseCheckerStatus;
	struct MouseChecker mouseChecker;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;	


	init_MouseChecker(&mouseChecker);
	init_gdtidt();
	init_pic();
	io_sti();
	FIFOBuffer_Init(&fifoBuffer, 32, keyBuf);
	FIFOBuffer_Init(&mourseFifoBuffer, 128, mouBuf);
	//设置CPU的开中断，CPU接收中断
	init_pit();
	io_out8(PIC0_IMR, 0xf9); 
	io_out8(PIC1_IMR, 0xef); 

	init_keyboard();
	enable_mouse();

	// 初始化内存管理
    totalMemory = memtest(0x00400000, 0xbfffffff);
	MemoryManagement_init(memoryManager);
	MemoryManagement_free(memoryManager, 0x00001000, 0x0009e000);/* free 0x00001000 - 0x0009efff */
	MemoryManagement_free(memoryManager, 0x00400000, totalMemory - 0x00400000);
	sprintf(memtest, "%dMB, %dKB", totalMemory/(1024*1024), MemoryManagement_current_free(memoryManager)/1024);
	 
	// layer hierarchy
//draw_cursor(bootinfo->vram, bootinfo->scrnx, 16, 16 , mx, my, cursorBuf, 16);
	init_color();
	shtctl = shtctl_init(memoryManager, bootinfo->vram, bootinfo->scrnx, bootinfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
	buf_back = (unsigned char *)MemoryManagement_alloc_page(memoryManager, bootinfo->scrnx * bootinfo->scrny);
	buf_win = (unsigned char *)MemoryManagement_alloc_page(memoryManager, 160 * 68);
	sheet_setbuf(sht_back, buf_back, bootinfo->scrnx, bootinfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 68, -1);
	init_screen(buf_back, bootinfo->scrnx, bootinfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	sheet_window(buf_win, 160, 68, "window");
	put_string8(buf_win, 160, COL8_000000, "Welcome to", 24, 28);
	put_string8(buf_win, 160, COL8_000000, "  Monk-OS", 24, 44);
	sheet_slide(sht_back, 0, 0);
	mx = (bootinfo->scrnx - 16) / 2;
	my = (bootinfo->scrny - 16) / 2;
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 70);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_mouse, 2);
	sheet_updown(sht_win, 1);
	sprintf(s, "(%3d, %3d)", mx, my);
	put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
	sheet_refresh(sht_back, 0, 0, bootinfo->scrnx, 48);

	put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, memtest, 0, 60);
	sheet_refresh(sht_back, 0, 60, 40*16, 76);
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
				draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 0, 0,  12*8,16);
				put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
				sheet_refresh(sht_back, 0, 0,  12*8,16);
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
					draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 0, 16,  12*8*3, 32);
					put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 16);
					sheet_refresh(sht_back, 0, 16,  12*8*3, 32);

					mx += mouseChecker.x;
					my -= mouseChecker.y;

					if(mx < 0)
						mx = 0;
					if(my < 0)
						my = 0;
					if(mx > bootinfo->scrnx - 1)
						mx = bootinfo->scrnx - 1 ;
					if(my > bootinfo->scrny - 1)
						my = bootinfo->scrny - 1;

					sprintf(s, "(%3d, %3d)", mx, my);
					draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 0, 0,12*8+12*8,16);
					put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
					sheet_refresh(sht_back, 0, 0, bootinfo->scrnx, 48);				
					sprintf(mouses, "x=%d y=%d",mx, my);
					draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 12*8+10, 0,  12*8+12*8+10,16);
					put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, mouses, 12*8+10, 0);
					sheet_refresh(sht_back, 12*8+10, 0,  12*8+12*8+10,16);
					sheet_slide(sht_mouse, mx, my);
				}		

			}
		}
	}
}
