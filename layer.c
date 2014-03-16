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
	ctl = (struct SHTCTL *)MemoryManagement_alloc_page(memManager, sizeof (struct SHTCTL));
	if (ctl == 0) {
		goto err;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; // we don't have any sheet now
	for (i = 0; i < MAX_SHEETS; i++) {
		ctl->sheets0[i].flags = 0; //mark it as unused.
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
void sheet_updown(struct SHTCTL *ctl, struct SHEET *sht, int height)
{
	int h, old = sht->height; //store layer's height firstly.

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
		} else { // height = -1, to hide it
			if (ctl->top > old) {
				// get down all other layers
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; //decrease top one's height
		}
		sheet_refresh(ctl); //refresh layers
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
		sheet_refresh(ctl);
	}
	return;
}

void sheet_refresh(struct SHTCTL *ctl)
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		for (by = 0; by < sht->bysize; by++) {
			vy = sht->vy0 + by;
			for (bx = 0; bx < sht->bxsize; bx++) {
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

//在当前的结构中移动图层
void sheet_slide(struct SHTCTL *ctl, struct SHEET *sht, int vx0, int vy0)
{
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { 
		sheet_refresh(ctl);
	}
	return;
}

void sheet_free(struct SHTCTL *ctl, struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(ctl, sht, -1); // hide it
	}
	sht->flags = 0; // mark as unused layer
}