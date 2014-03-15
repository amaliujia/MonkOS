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

int MemoryManagement_free(struct MemoryManager *memManager, unsigned int address, unsigned int size)
{
	int i, j;
	// sort by address
	for(i = 0; i < memManager->free; i++)
	{
		if (memManager->segmentInfo[i].address > address)
		{
			break;
		}
	}
	// segmentInfo[i-1] < address < segmentInfo[i]
	if(i > 0)
	{
		if (memManager->segmentInfo[i-1].address + memManager->segmentInfo[i-1].size == address)	
		{
			memManager->segmentInfo[i-1].size += size;
		}
		if (i < memManager->free)
		{
			if (address + size == memManager->segmentInfo[i].address)
			{
				memManager->segmentInfo[i-1].size += memManager->segmentInfo[i].size;
				// delete segmentInfo[i]
				memManager->free--;
				for (; i < memManager->free; ++i)
				{
					memManager->segmentInfo[i] = memManager->segmentInfo[i+1];
				}
			}
			return 0;
		}
	}
	// this address is largest, cannot merge with all segments
	if(i < memManager->free)
	{
		if (address + size == memManager->segmentInfo[i].address)
		{
			memManager->segmentInfo[i].address = address;
			memManager->segmentInfo[i].size += size;
			return 0;
		}
	}

	if(memManager->free < MEMMAN_FREES)
	{
		//segmentInfo[i]
		for (j = memManager->free; j > i; j--)
		{
		 	memManager->segmentInfo[j] = memManager->segmentInfo[j-1];
		}
		memManager->free++;
		if (memManager->maxFree < memManager->free)
		{
		 	memManager->maxFree = memManager->free;
		}
		memManager->segmentInfo[i].address = address;
		memManager->segmentInfo[i].size = size;
		return 0; 
	}
	// run out of slot
	memManager->losts++;
	memManager->lostSize += size;
	return -1;
}

unsigned int MemoryManagement_current_free(struct MemoryManager *memManager)
{
	int i;
	unsigned int total = 0;
	for (i = 0; i < memManager->free; ++i)
	{
		total += memManager->segmentInfo[i].size;
	}
	return total;
}

unsigned int MemoryManager_alloc_page(struct MemoryManager *memManager, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = MemoryManagement_alloc(memManager, size);
	return a;
}

int MemoryManagement_free_page(struct MemoryManager *memManager, unsigned int address, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	MemoryManagement_free(memManager, address, size);
	return i;
}