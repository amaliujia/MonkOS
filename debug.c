#include "bootpack.h"

void process_show()
{
    static int count = 0;
    char s[40];
    struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
    sprintf(s, "count= %010d",count++);
    draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_840000, 0, 120,  300,120+16);
    put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 120);
    return;
}

void process_show_buddy()
{
    static int count = 0;
    char s[40];
    struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
    sprintf(s, "count= %010d",count++);
    draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_840000, 0, 136,  300,120+32);
    put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 136);
    return;
}

void process_show_string(struct SHEET *sheet, char *string)
{
    put_string_package(sheet, 0, 120, COL8_FFFFFF, COL8_840000, string, 20);
}

void FIFOBuffer_show(struct FIFOBuffer *fifoBuffer)
{
    static int count = 0;
    char s[100];
    struct BOOTINFO *bootinfo = (struct BOOTINFO *)BOOTINFO_ADDR;
    
    sprintf(s, "size :%d space:%d start:%d end:%d flag:%d", fifoBuffer->size,fifoBuffer->space,fifoBuffer->start,fifoBuffer->end,fifoBuffer->flags);
    if(count == 0){
        draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 60,  320,16);
        put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 60);
        count++;
        
    }else if (count == 1)
    {
        draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 80,  320,16);
        put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 80);
        count++;
    }else{
        draw_box8(bootinfo->vram, bootinfo->scrnx, COL8_000000, 0, 100,  320,16);
        put_string8(bootinfo->vram, bootinfo->scrnx, COL8_FFFFFF, s, 0, 100);
        count++;
    }
    return;
}
