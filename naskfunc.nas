;naskfunc-assembly level function
;TAB=4

[FORMAT "WCOFF"]			;Obj file pattern
[BITS 32]					;32-bit machine code

;set obj info
[FILE "naskfunc.nas"]		;file name
	
		GLOBAL	_io_hlt		;func name which can be linked
							;with C

[SECTION .text]				;imply real part of func
_io_hlt:					;void io_hlt(void);
		HLT
		RET					;return