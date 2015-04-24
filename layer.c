#include "bootpack.h"

// struct SHTCTL *SHTCTL_init(struct MemoryManager* memManager, unsigned char *vram, int xsize, int ysize)
// {
// 	struct SHTCTL *ctl;
// 	int i;
// 	ctl = (struct SHTCTL *)MemoryManagement_alloc_page(memManager, sizeof(struct SHTCTL));
// 	if (ctl == 0)
// 	{
// 		goto error;
// 	}
// 	ctl->vram = vram;
// 	ctl->xsize = xsize;
// 	ctl->ysize = ysize;
// 	ctl->top = -1;
// 	for (i = 0; i < MAX_SHEETS; ++i)
// 	{
// 		ctl->sheets0[i].flags = 0;
// 	}

// error:
// 	return ctl;
// }

#define SHEET_USE		1

struct SHTCTL *shtctl_init(struct MemoryManager *memManager, unsigned char *vram, int xsize, int ysize)
{
    struct SHTCTL *ctl;
    int i;
    // we alloc it, we can the whole thing
    // include two arrays of layer
    ctl = (struct SHTCTL *) MemoryManagement_alloc_page(memManager, sizeof (struct SHTCTL));
    if (ctl == 0) {
        goto err;
    }
    ctl->map = (unsigned char *) MemoryManagement_alloc_page(memManager, xsize * ysize);
    if (ctl->map == 0) {
        MemoryManagement_free_page(memManager, (int)ctl, sizeof (struct SHTCTL));
        goto err;
    }
    ctl->vram = vram;
    ctl->xsize = xsize;
    ctl->ysize = ysize;
    ctl->top = -1; // we don't have any sheet now
    for (i = 0; i < MAX_SHEETS; i++) {
        ctl->sheets0[i].flags = 0; //mark it as unused.
        ctl->sheets0[i].ctl = ctl;
    }
err:
    return ctl;
}

struct SHEET *sheet_alloc(struct SHTCTL *ctl)
{
    struct SHEET *sht;
    int i;
    for (i = 0; i < MAX_SHEETS; i++) {
        if (ctl->sheets0[i].flags == 0) {
            sht = &ctl->sheets0[i]; // get address of this available layer
            sht->flags = SHEET_USE; //mark as used
            sht->height = -1; // haven't decided what height it should be
            return sht;
        }
    }
    return 0;	//no available sheet
}

//initialize the layer what we alloc just now
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
    sht->buf = buf;
    sht->bxsize = xsize;
    sht->bysize = ysize;
    sht->col_inv = col_inv;
    return;
}

// reorder layer hierarchy
void sheet_updown(struct SHEET *sht, int height)
{
    int h, old = sht->height; //store layer's height firstly.
    struct SHTCTL *ctl = sht->ctl;
    //如果指定的高度过高或高低，则修正
    if (height > ctl->top + 1) {
        height = ctl->top + 1;
    }
    if (height < -1) {
        height = -1;
    }
    sht->height = height; // set new height
    
    // lower than before
    if (old > height) {
        if (height >= 0) {
            //reorder
            for (h = old; h > height; h--) {
                ctl->sheets[h] = ctl->sheets[h - 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0+sht->bxsize, sht->vy0+sht->bysize, height+1);
        } else { // height = -1, to hide it
            if (ctl->top > old) {
                // get down all other layers
                for (h = old; h < ctl->top; h++) {
                    ctl->sheets[h] = ctl->sheets[h + 1];
                    ctl->sheets[h]->height = h;
                }
            }
            ctl->top--; //decrease top one's height
            sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0); //refresh layers
        }
    } else if (old < height) {	// higher than before
        if (old >= 0) {
            for (h = old; h < height; h++) {
                ctl->sheets[h] = ctl->sheets[h + 1];
                ctl->sheets[h]->height = h;
            }
            ctl->sheets[height] = sht;
        } else {
            for (h = ctl->top; h >= height; h--) {
                ctl->sheets[h + 1] = ctl->sheets[h];
                ctl->sheets[h + 1]->height = h + 1;
            }
            ctl->sheets[height] = sht;
            ctl->top++;
        }
        sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
    }
    return;
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{
    int h, bx, by, vx, vy, bx0, by0, bx1, by1;
    unsigned char *buf, c, *vram = ctl->vram;
    struct SHEET *sht;
    if(vx0 < 0) {vx0 = 0;}
    if(vy0 < 0) {vy0 = 0;}
    if(vx1 > ctl->xsize) {vx1 = ctl->xsize;}
    if(vy1 > ctl->ysize) {vy1 = ctl->ysize;}
    for (h = h0; h <= ctl->top; h++) {
        sht = ctl->sheets[h];
        buf = sht->buf;
        bx0 = vx0 - sht->vx0;
        by0 = vy0 - sht->vy0;
        bx1 = vx1 - sht->vx0;
        by1 = vy1 - sht->vy0;
        if (bx0 < 0) { bx0 = 0; }
        if (by0 < 0) { by0 = 0; }
        if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
        if (by1 > sht->bysize) { by1 = sht->bysize; }
        for (by = by0; by < by1; by++) {
            vy = sht->vy0 + by;
            for (bx = bx0; bx < bx1; bx++) {
                vx = sht->vx0 + bx;
                c = buf[by * sht->bxsize + bx];
                if (c != sht->col_inv) {
                    vram[vy * ctl->xsize + vx] = c;
                }
            }
        }
    }
    return;
}
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
    if (sht->height >= 0) {
        sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height);
    }
    return;
}

//在当前的结构中移动图层
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
    int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
    sht->vx0 = vx0;
    sht->vy0 = vy0;
    if (sht->height >= 0) {
        sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
        sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
    }
    return;
}

void sheet_free(struct SHEET *sht)
{
    if (sht->height >= 0) {
        sheet_updown(sht, -1); // hide it
    }
    sht->flags = 0; // mark as unused layer
}

void sheet_error(unsigned char *buf, int xsize, int ysize)
{
    draw_box8(buf, xsize, COL8_008484,  0,         0,          xsize -  1, ysize);
}
void sheet_window(unsigned char *buf, int xsize, int ysize, char *title)
{
    static char closebtn[14][16] = {
        "OOOOOOOOOOOOOOO@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQQQ@@QQQQQ$@",
        "OQQQQQ@@@@QQQQ$@",
        "OQQQQ@@QQ@@QQQ$@",
        "OQQQ@@QQQQ@@QQ$@",
        "OQQQQQQQQQQQQQ$@",
        "OQQQQQQQQQQQQQ$@",
        "O$$$$$$$$$$$$$$@",
        "@@@@@@@@@@@@@@@@"
    };
    int x, y;
    char c;
    draw_box8(buf, xsize, COL8_C6C6C6, 0,         0,         xsize - 1, 0        );
    draw_box8(buf, xsize, COL8_FFFFFF, 1,         1,         xsize - 2, 1        );
    draw_box8(buf, xsize, COL8_C6C6C6, 0,         0,         0,         ysize - 1);
    draw_box8(buf, xsize, COL8_FFFFFF, 1,         1,         1,         ysize - 2);
    draw_box8(buf, xsize, COL8_848484, xsize - 2, 1,         xsize - 2, ysize - 2);
    draw_box8(buf, xsize, COL8_000000, xsize - 1, 0,         xsize - 1, ysize - 1);
    draw_box8(buf, xsize, COL8_C6C6C6, 2,         2,         xsize - 3, ysize - 3);
    draw_box8(buf, xsize, COL8_000084, 3,         3,         xsize - 4, 20       );
    draw_box8(buf, xsize, COL8_848484, 1,         ysize - 2, xsize - 2, ysize - 2);
    draw_box8(buf, xsize, COL8_000000, 0,         ysize - 1, xsize - 1, ysize - 1);
    put_string8(buf, xsize, COL8_FFFFFF, title, 24, 4);
    for (y = 0; y < 14; y++) {
        for (x = 0; x < 16; x++) {
            c = closebtn[y][x];
            if (c == '@') {
                c = COL8_000000;
            } else if (c == '$') {
                c = COL8_848484;
            } else if (c == 'Q') {
                c = COL8_C6C6C6;
            } else {
                c = COL8_FFFFFF;
            }
            buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
        }
    }
    return;
}

void make_textbox(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
    int x1 = x0 + sx, y1 = y0 + sy;
    draw_box8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
    draw_box8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
    draw_box8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
    draw_box8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
    draw_box8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
    draw_box8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
    draw_box8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
    draw_box8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
    draw_box8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
    return;
}