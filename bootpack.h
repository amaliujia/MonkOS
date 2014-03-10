#include <stdio.h>
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
void asm_inthandler21(void);
void asm_inthandler2c(void);


//Keyboard
struct KeyboardBuffer
{
	unsigned char data, flag;
};
//init keyboard buffer
void init_KeyboardBuffer(struct KeyboardBuffer *keybuf);



