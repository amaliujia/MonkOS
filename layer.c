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