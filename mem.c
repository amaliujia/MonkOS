#include "bootpack.h"



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