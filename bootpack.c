#include "bootpack.h"

//显卡计算公式：0xa0000 + 行地址 + 列地址 * 320

extern struct FIFOBuffer fifoBuffer;
extern struct FIFOBuffer mourseFifoBuffer;
extern struct TimerCTL timerCTL;
//struct SHEET *sht_error;
void HariMain(void)
{

	struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
	char s[40], keyBuf[32], mouBuf[128];
	char memTest[40];
	char mouses[40];
	int mx, my;
	int i = 0;
	int cursor_location, cursor_color, task_b_esp;
	unsigned int totalMemory;
	struct MemoryManager *memoryManager = (struct memoryManager *)MEMMAN_ADDR;
	struct FIFOBuffer timerfifo, timerfifo2, timerfifo3;
	char timerBuf[8];
	struct Timer *timer1, *timer2, *timer3;
	int mouseCheckerStatus;
	struct MouseChecker mouseChecker;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;
	struct TaskStatusSegment tss_a, tss_b;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

	init_MouseChecker(&mouseChecker);
	init_gdtidt();
	init_pic();
	io_sti();
	FIFOBuffer_Init(&fifoBuffer, 32, keyBuf);
	FIFOBuffer_Init(&mourseFifoBuffer, 128, mouBuf);
	//设置CPU的开中断，CPU接收中断
	init_pit();
	init_keyboard();
	enable_mouse();
	io_out8(PIC0_IMR, 0xf8); 
	io_out8(PIC1_IMR, 0xef);


	FIFOBuffer_Init(&timerfifo, 8, timerBuf);
	timer1 = Timer_alloc();
	Timer_init(timer1, &timerfifo, 10);
	Timer_SetTimer(timer1, 1000);
//	FIFOBuffer_Init(&timerfifo2, 16, timerBuf2);
	timer2 = Timer_alloc();
	Timer_init(timer2, &timerfifo, 3);
	Timer_SetTimer(timer2, 300);
	//FIFOBuffer_Init(&timerfifo3, 8, timerBuf3);
 	timer3 = Timer_alloc();
 	Timer_init(timer3, &timerfifo, 1);
 	Timer_SetTimer(timer3, 50);

	// 初始化内存管理
	//tss_a 也就是本文件的程序的段状态 
    totalMemory = memtest(0x00400000, 0xbfffffff);
	MemoryManagement_init(memoryManager);
	MemoryManagement_free(memoryManager, 0x00001000, 0x0009e000);/* free 0x00001000 - 0x0009efff */
	MemoryManagement_free(memoryManager, 0x00400000, totalMemory - 0x00400000);
	sprintf(memtest, "%dMB, %dKB", totalMemory/(1024*1024), MemoryManagement_current_free(memoryManager)/1024);
	 
	// layer hierarchy
	init_color();
	shtctl = shtctl_init(memoryManager, bootinfo->vram, bootinfo->scrnx, bootinfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	sht_win = sheet_alloc(shtctl);
//	sht_error = sheet_alloc(shtctl);
	buf_back = (unsigned char *)MemoryManagement_alloc_page(memoryManager, bootinfo->scrnx * bootinfo->scrny);
//	buf_error = (unsigned char *)MemoryManagement_alloc_page(memoryManager, bootinfo->scrnx * bootinfo->scrny);
	buf_win = (unsigned char *)MemoryManagement_alloc_page(memoryManager, 160 * 52);
	sheet_setbuf(sht_back, buf_back, bootinfo->scrnx, bootinfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);
//	sheet_setbuf(sht_error, buf_error, bootinfo->scrnx, bootinfo->scrny, -1);
	init_screen(buf_back, bootinfo->scrnx, bootinfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);
	sheet_window(sht_win->buf, 160, 52, "window");
	make_textbox(sht_win, 8, 28, 144, 16, COL8_FFFFFF);
	cursor_location = 8;
	cursor_color = COL8_FFFFFF;
	//sheet_error(buf_error, bootinfo->scrnx, bootinfo->scrny);
	sheet_slide(sht_back, 0, 0);
	mx = (bootinfo->scrnx - 16) / 2;
	my = (bootinfo->scrny - 16) / 2;
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_win, 80, 72);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_win, 1);
	sheet_updown(sht_mouse, 2);
	//sheet_updown(sht_error, 1);
	sprintf(s, "(%3d, %3d)", mx, my);
	put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
	sheet_refresh(sht_back, 0, 0, bootinfo->scrnx, 48);
	put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, memtest, 0, 44);
	sheet_refresh(sht_back, 0, 44, 40*16, 60);

	tss_a.ldtr = 0;
	tss_a.iomap = 0x40000000;
	tss_b.ldtr = 0;
	tss_b.iomap = 0x40000000;
	set_segmdesc(gdt+3, 103, (int)&tss_a, AR_TSS32);
	set_segmdesc(gdt+4, 103, (int)&tss_b, AR_TSS32);
	load_tr(3*8);
	task_b_esp = MemoryManagement_alloc_page(memoryManager, 64 * 1024) + 64 * 1024;
	tss_b.eip = (int)&task_b_main;
	tss_b.eflags = 0x00000202; // IF=1
	tss_b.eax = 0;
	tss_b.eax = 0;
	tss_b.ecx = 0;
	tss_b.edx = 0;
	tss_b.ebx = 0;
	tss_b.esp = task_b_esp;
	tss_b.ebp = 0;
	tss_b.esi = 0;
	tss_b.edi = 0;
	tss_b.es = 1 * 8;
	tss_b.cs = 2 * 8;
	tss_b.ss = 1 * 8;
	tss_b.ds = 1 * 8;
	tss_b.fs = 1 * 8;
	tss_b.gs = 1 * 8;
	for (;;)
	{
		io_cli();
		if (FIFOBuffer_Status(&fifoBuffer) + FIFOBuffer_Status(&mourseFifoBuffer) + FIFOBuffer_Status(&timerfifo) == 0)
		{
			io_stihlt();
		}else{
			if(FIFOBuffer_Status(&fifoBuffer) != 0){
				i = FIFOBuffer_Get(&fifoBuffer);
				io_sti();
				sprintf(s, "%02X", i);
				draw_box8(sht_back->buf, bootinfo->scrnx, COL8_008484, 0, 0,  12*8,16);
				put_string8(sht_back->buf, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
				sheet_refresh(sht_back, 0, 0,  12*8,16);
				if(i < 0x54){
					if(i != 0x0e && cursor_location < 144){
							if(chartable[i] != 0){
								s[0] = chartable[i];
								s[1] = 0;
								put_string_package(sht_win, cursor_location, 28, COL8_000000, COL8_FFFFFF, s, 1);
								cursor_location += 8;
						}
					}else if(i == 0x0e && cursor_location > 8){
							put_string_package(sht_win, cursor_location, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_location -= 8;
					}else if(i == 0x0e && cursor_location == 8){
							put_string_package(sht_win, cursor_location, 28, COL8_000000, COL8_FFFFFF, " ", 1);
					}
				}
			}else if(FIFOBuffer_Status(&mourseFifoBuffer) != 0){
				//process_show();
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
					// draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 0, 16,  12*8*3, 32);
					// put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 16);
					// sheet_refresh(sht_back, 0, 16,  12*8*3, 32);

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

					// sprintf(s, "(%3d, %3d)", mx, my);
					// draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 0, 0,12*8+12*8,16);
					// put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, s, 0, 0);
					// sheet_refresh(sht_back, 0, 0, bootinfo->scrnx, 48);				
					sprintf(mouses, "x=%d y=%d",mx, my);
					draw_box8(buf_back, bootinfo->scrnx, COL8_008484, 12*8+10, 0,  12*8+12*8+10,16);
					put_string8(buf_back, bootinfo->scrnx, COL8_FFFFFF, mouses, 12*8+10, 0);
					sheet_refresh(sht_back, 12*8+10, 0,  12*8+12*8+10,16);
					sheet_slide(sht_mouse, mx, my);
					if ((mouseChecker.btn & 0x01) != 0)
					{
						sheet_slide(sht_win, mx-80, my-8);
					}
				}		
			}else if(FIFOBuffer_Status(&timerfifo) != 0){
				i = FIFOBuffer_Get(&timerfifo);
				io_sti();
				if(i == 10){
				put_string8(buf_back, bootinfo->scrnx, COL8_840000, "10Sec", 0, 80);
				sheet_refresh(sht_back, 0, 80, 56, 96);
				}else if(i == 3){
				put_string8(buf_back, bootinfo->scrnx, COL8_840000, "3Sec", 0, 96);
				sheet_refresh(sht_back, 0, 96, 56, 112);
				ProcessSwitch();
				}else if(i == 1){
				put_string8(buf_back, bootinfo->scrnx, COL8_840000, "0.5Sec", 0, 112);
				sheet_refresh(sht_back, 0, 112, 56, 128);
				}	
			}
		}
	}
}


void task_b_main(void)
{
	process_show();
	for (;;) 
	{ 
		io_hlt(); 
	}
}