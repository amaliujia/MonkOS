#include "bootpack.h"

unsigned int memtest(unsigned int start ,unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 判断CPU是386还是486及以后 */
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0) { /* 386‚AC = 0 */
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;  //AC-bit = 0 
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; /* forbid cache*/
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; /* allow cache*/
		store_cr0(cr0);
	}
	return i;
}
// unsigned int memtest_sub(unsigned int start, unsigned int end)
// {
// 	unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
// 	for (i = start; i <= end; i += 0x1000) {
// 		p = (unsigned int *) (i + 0xffc);
// 		old = *p;			/* record old value */
// 		*p = pat0;			/* assign test value */
// 		*p ^= 0xffffffff;	/* 取反运算 */
// 		if (*p != pat1) {	/* if is not equal to pat1 */
// not_memory:
// 			*p = old;
// 			break;
// 		}
// 		*p ^= 0xffffffff;	/* 再次反转 */
// 		if (*p != pat0) {	/* if not change back*/
// 			goto not_memory;
// 		}
// 		*p = old;			/* Whatever, get back old value */
// 	}
// 	return i;
// }


void MemoryManagement_init(struct MemoryManager *memManager)
{
	memManager->free = 0;
	memManager->maxFree = 0;
	memManager->lostSize = 0;
	memManager->losts = 0;
	return;
}
unsigned int MemoryManagement_alloc(struct MemoryManager *memManager, unsigned int size)
{
	int i;
	unsigned int addressResult;
	for (i = 0; i < memManager->free; ++i)
	{
		if (memManager->segmentInfo[i].size >= size)
		{
			addressResult = memManager->segmentInfo[i].address;
			memManager->segmentInfo[i].address += size;
			memManager->segmentInfo[i].size -= size;
	
			if (memManager->segmentInfo[i].size == 0)
			{
				i++;
				for (i; i < memManager->free; ++i)
				{
					memManager->segmentInfo[i-1] = memManager->segmentInfo[i]  ;
				}
				memManager->free -= 1;
			}
			return addressResult; 
	    }
	}
	return 0;
}

// struct SegmentInfo
// {
// 	unsigned int address, size;
// };

// struct MemoryManager	
// {
// 	// free: segments available
// 	// maxFree: frees' historical maximum number
// 	// lostSize:total size of memory released wrongly
// 	// losts: failed times
// 	struct SegmentInfo segmentInfo[1000];
// 	int free, maxFree, lostSize, losts;
// };
int MemoryManagement_free(struct MemoryManager *memManager, unsigned int address, unsigned int size)
{
	
}