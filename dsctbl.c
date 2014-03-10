#include "bootpack.h"


void init_gdtidt()
{
	// 0x00270000, 打算把0x270000 - 0x27ffff 留给GDT用. 8B per segmdesc
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	// 0x0026f800 - 0x0026ffff 留给IDT用
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *)ADR_IDT;
	// In the meantime, we put bootpack.h into 0x280000 - 0x2fffff.
	int i;
	/* initialize GDT */
	for (i = 0; i < LIMIT_GDT/8; ++i)
	{
		set_segmdesc(gdt+i, 0, 0, 0);
	}
	// seg1是4GB，即32位下CPU能管理的最大段，基址从0开始
	set_segmdesc(gdt+1, 0xffffffff, 0x00000000, AR_DATA32_RW);
	//seg2是512KB, 基址是0x280000，从来存bootpack.hrb
	set_segmdesc(gdt+2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER);
	//operator GDTR register
	load_gdtr(LIMIT_GDT, ADR_GDT);

	/*initialize IDT */
	for (i = 0; i < LIMIT_IDT/8; ++i)
	{
		set_gatedesc(idt+i, 0, 0, 0);
	}
	//operate LDTR register
	load_idtr(LIMIT_IDT, ADR_IDT);

	/*
		Set IDT
	*/
	// set_gatedesc(idt+0x21, (int) asm_inthandler21, 2*8, AR_INTGATE32);
	// set_gatedesc(idt+0x2c, (int) asm_inthandler2c, 2*8, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 * 8, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 * 8, AR_INTGATE32);

	return;
}

/*
	段属性，也就是段的访问属性，access_right or ar.
	ar high 4 bit is 扩展访问权，GD00, G means Gbit，D means 段的模式，1代指32位，0代指16位
	ar low 8 bit. 
	0x00: 未使用的记录表(descriptor table)
	0x92：系统专用段，可读写，不可执行
	0x9a：系统专用，可执行段，可读不可写
	0xf2：应用程序用，可读写段，不可执行
	0xfa：应用程序段，可执行段，可读不可写
*/
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 CPU将段的每一位解释为一页*/
		//段上限只有20位，也就是1M.但由于1page=4KB，1M*4KB=4GB
		limit /= 0x1000;
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
	return;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
	return;
}
