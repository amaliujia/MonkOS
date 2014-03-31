#include <stdio.h>

/*
	Error definition
*/
#define OS_OK 						0;
#define OS_TIMER_ALLOC_FAIL			1;


//fundamental functions 
void io_hlt(void);
void write_mem8(int addr, int data);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

//Boot info
static char chartable[0x54] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.'
};

#define BOOTINFO_ADDR 0x0ff0

struct BOOTINFO
{
	char cyls, leds, vmode, reserve;
	short scrnx, scrny;
	char *vram;
};


//screen
#define COL8_000000		0   //黑
#define COL8_FF0000		1   //亮红
#define COL8_00FF00		2	//亮绿
#define COL8_FFFF00		3	//亮黄
#define COL8_0000FF		4	//亮蓝
#define COL8_FF00FF		5	//亮紫
#define COL8_00FFFF		6	//浅亮蓝
#define COL8_FFFFFF		7	//白
#define COL8_C6C6C6		8 	//亮灰
#define COL8_840000		9	//暗红
#define COL8_008400		10	//暗绿
#define COL8_848400		11	//暗黄
#define COL8_000084		12	//暗蓝
#define COL8_840084		13	//暗紫
#define COL8_008484		14	//浅暗蓝
#define COL8_848484		15	//暗灰

//init related parameters of pallet
void init_color();
//set color pallet
void set_color(int start, int end, unsigned char *rgb);
//init screen
void init_screen(char *vram, int xsize, int ysize);
//draw func, draw a rectangle on screen
void draw_box8(unsigned char *vram, int xsize, unsigned char c, int xs, int ys, int xe, int ye);
//draw 8bit long font
void put_font8(unsigned char *vram, int xsize, unsigned char fontColor, char *c, int x, int y);
//draw string
void put_string8(unsigned char *vram, int xsize, unsigned char fontColor, char *str, int x, int y);

//IO

//init mouse cursor
void init_mouse_cursor8(char *mouse, char backgourdColor);
//draw cursor and its backgourd
void draw_cursor(char *vram, int xsize, int cursorXSize, int cursorYSize, int startPointX, int startPointY, char *cursorBuf, int backgourdXSize);


// GDT
//GDT descriptor structure
struct SEGMENT_DESCRIPTOR
{
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

void init_gdtidt();
void load_gdtr(int limit, int addr);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);

//IDT
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e
#define AR_TSS32		0x0089

//IDT descriptor structure
struct GATE_DESCRIPTOR
{
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void load_idtr(int limit, int addr);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);


//PIC

//define some constants 
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

//initialize PIC
void init_pic(void);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);



//Keyboard and mouse
struct KeyboardBuffer
{
	unsigned char data[32];
	int start,end,len; 

};
void wait_KBC_sendready(void);
void init_keyboard(void);
void enable_mouse(void);

struct MouseChecker{
	char mouseCheckBuffer[3];
	int phase;
	int x, y, btn;
};

// mouse buffer methods
void init_MouseChecker(struct MouseChecker *checker);
int MouseChecker_Job(struct MouseChecker *checker, int i);

// Older version of keyboard buffer, useless now
//init keyboard buffer
void init_KeyboardBuffer(struct KeyboardBuffer akeyboardBuffer);
char KeyboardBuffer_Remove(struct KeyboardBuffer akeyboardBuffer);
void KeyboardBuffer_Add(char data, struct KeyboardBuffer akeyboardBuffer);

//universal IO buffer
struct TaskStatusSegment
{
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	int eip, eflags, eax, ebx, ecx, edx, esp, ebp, esi, edi;
	int es, cs, ss, ds, fs, gs;
	int ldtr, iomap; 
};

struct Process{
	int segmentNo, flags;
	struct TaskStatusSegment status;
};

struct FIFOBuffer
{
	unsigned char *buf;
	int start, end, size, space, flags;
	struct Process *process;	
};

// UIOBuffer init func
void FIFOBuffer_Init(struct FIFOBuffer *fifoBuffer, int size, unsigned char *buf);
// UIOBuffer init with func
void FIFOBuffer_Init_Process(struct FIFOBuffer *fifoBuffer, int size, unsigned char *buf, struct Process* process);
//UIOBuffer add func
int FIFOBuffer_Add(struct FIFOBuffer *fifoBuffer, unsigned char data);
//UIOBuffer get func
int FIFOBuffer_Get(struct FIFOBuffer *fifoBuffer);
//UIOBuffer status func
int FIFOBuffer_Status(struct FIFOBuffer *fifoBuffer);

/*
Memory Management
*/
#define EFLAGS_AC_BIT	0x00040000
#define CR0_CACHE_DISABLE	0x60000000

int load_cr0(void);
void store_cr0(int cr0);

unsigned int memtest(unsigned int start ,unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

#define MEMORYMANAGEMENT	0x003c0000
#define MEMMAN_FREES		4090
#define MEMMAN_ADDR			0x003c0000

struct SegmentInfo
{
	unsigned int address, size;
};

struct MemoryManager	
{
	// free: segments available
	// maxFree: frees' historical maximum number
	// lostSize:total size of memory released wrongly
	// losts: failed times
	struct SegmentInfo segmentInfo[1000];
	int free, maxFree, lostSize, losts;
};
void MemoryManagement_init(struct MemoryManager *memManager);
//void MemoryManagement_setFree(struct MemoryManager *memManager, unsigned int address, unsigned int size);
unsigned int MemoryManagement_alloc(struct MemoryManager *memManager, unsigned int size);
int MemoryManagement_free(struct MemoryManager *memManager, unsigned int address, unsigned int size);
unsigned int MemoryManagement_current_free(struct MemoryManager *memManager);
unsigned int MemoryManagement_alloc_page(struct MemoryManager *memManager, unsigned int size);
int MemoryManagement_free_page(struct MemoryManager *memManager, unsigned int address, unsigned int size);


// screen layers
#define MAX_SHEETS		256
struct SHEET {
	unsigned char *buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};
struct SHTCTL {
	unsigned char *vram, *map;
	int xsize, ysize, top;
	struct SHEET *sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MemoryManager *memManager, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);
void sheet_error(unsigned char *buf, int xsize, int ysize);
void sheet_window(unsigned char *buf, int xsize, int ysize, char *title);
void make_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
//a package contains drawing background, drawing string, layer refreshment
void put_string_package(struct SHEET *sheet, int x, int y, int wordColor, int backgroundColor, char *string, int length);

/*
	timer
*/
#define PIT_CTRL	0x0043
#define PIT_CNT0	0x0040
#define MAX_TIMER 500

struct Timer
{
	unsigned int timeout;
	//record state of this timer
	unsigned int flag;
	//记录离超时还有多少时间
	struct FIFOBuffer *fifo;
	int data;
};

struct TimerCTL
{
	//开机计时
	unsigned int count;
	//next possible time out timer
	unsigned int next;
	//how many timers are active
	unsigned int active;
	//timers
	struct Timer timers[MAX_TIMER];
	//active timers
	struct Timer *timersInActive[MAX_TIMER];
};

// struct FIFOBufferInteger
// {
// 	int *buf;
// 	int start, end, size, space, flags;
// };
// //UIOBuffer add 32bits func
// void FIFOBuffer_Init_Integer(struct FIFOBufferInteger *fifoBuffer, int size, int *buf);
// int FIFOBuffer_ADD_Integer(struct FIFOBufferInteger *fifoBuffer, int data);
// int FIFOBuffer_Get_Integer(struct FIFOBufferInteger *fifoBuffer);
// int FIFOBuffer_Status_Integer(struct FIFOBufferInteger *fifoBuffer);


void init_pit(void);
struct Timer* Timer_alloc(void);
void Timer_free(struct Timer *timer);
int Timer_init(struct Timer *timer, struct FIFOBuffer *fifo, unsigned char data);
int Timer_SetTimer(struct Timer *timer, unsigned int timeout);

//old version of settimer func
//void settimer(unsigned int timeout, struct FIFOBuffer *fifo, unsigned char data);

/*
multi-processes
*/

void load_tr(int tr);
void ProcessSwitch(int eip, int cs);

#define MAX_TASKS	1000
#define TASK_GDT0	3



struct ProcessCTL{
	int online;
	int runningnow;
	struct Process *processes[MAX_TASKS];
	struct Process allProcess[MAX_TASKS];
};

struct Process *Process_init(struct MemoryManager *memoryManager);
struct Process *Process_alloc();
void Process_run(struct Process *process);
void Process_switch(void);
void Process_sleep(struct Process* process);

/*
Debug func
*/
void process_show();
void process_show_buddy();
void FIFOBuffer_show(struct FIFOBuffer *fifoBuffer);
void process_show_string(struct SHEET *sheet, char *string);

