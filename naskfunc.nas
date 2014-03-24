;naskfunc-assembly level function
;TAB=4

[FORMAT "WCOFF"]			;Obj file pattern
[INSTRSET "i486p"]			;specific ISA
							;that is 32-bit set
							;so registers like EAX can be used
[BITS 32]					;32-bit machine code

;set obj info
[FILE "naskfunc.nas"]		;file name
	
		GLOBAL	_io_hlt		;func name which can be linked
							;with C
	    GLOBAL  _write_mem8	;

	    GLOBAL _io_cli		;forbid interrupt, set permission 					  ;flag 0
	    GLOBAL _io_sti		;allow interrupt, set flag 1
	    GLOBAL _io_stihlt	;
	    GLOBAL _io_in8		;
	    GLOBAL _io_in16		;
	    GLOBAL _io_in32		;
	    GLOBAL _io_out8		;
	    GLOBAL _io_out16	;
	    GLOBAL _io_out32	;
	    GLOBAL _io_load_eflags	;record interrupt permission
	    						;flag 
	    GLOBAL _io_store_eflags	;recover interrupt permission
	    						;flag 
	    GLOBAL	_load_gdtr	;operate GDTR 
	    GLOBAL	_load_idtr	;operate IDTR
	    GLOBAL	_load_cr0, _store_cr0 ;store CPU flag indicating cahce
	    GLOBAL 	_asm_inthandler21 ;interrupt handler for keyboard which is 0x21.
	    GLOBAL	_asm_inthandler2c
	    GLOBAL  _asm_inthandler27
	    GLOBAL  _asm_inthandler20 ;timer interrupt handler
	    GLOBAL	_memtest_sub
	    EXTERN _inthandler21, _inthandler2c, _inthandler27, _inthandler20
	    GLOBAL _load_tr
	    GLOBAL _ProcessSwitch 

[SECTION .text]				;imply real part of func
_io_hlt:					;void io_hlt(void);
		HLT
		RET					;return

_write_mem8:				;void write_mem8(int addr, int
							;data)
		MOV ECX,[ESP+4]		;[ESP+4] save first param
		MOV AL,[ESP+8]		;[ESP+8] save second param
		MOV [ECX],AL 		;move data into target addr
		RET
_io_cli:	; void io_cli(void);
		CLI
		RET

_io_sti:	; void io_sti(void);
		STI
		RET

_io_stihlt:	; void io_stihlt(void);
		STI
		HLT
		RET

_io_in8:	; int io_in8(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AL,DX
		RET

_io_in16:	; int io_in16(int port);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,0
		IN		AX,DX
		RET

_io_in32:	; int io_in32(int port);
		MOV		EDX,[ESP+4]		; port
		IN		EAX,DX
		RET

_io_out8:	; void io_out8(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		AL,[ESP+8]		; data
		OUT		DX,AL
		RET

_io_out16:	; void io_out16(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,AX
		RET

_io_out32:	; void io_out32(int port, int data);
		MOV		EDX,[ESP+4]		; port
		MOV		EAX,[ESP+8]		; data
		OUT		DX,EAX
		RET

_io_load_eflags:	; int io_load_eflags(void);
		PUSHFD		; PUSH EFLAGS
		POP		EAX
		RET

_io_store_eflags:	; void io_store_eflags(int eflags);
		MOV		EAX,[ESP+4]
		PUSH	EAX
		POPFD		; POP EFLAGS
		RET

; 将段上限和地址值赋给GDTR48位寄存器。
_load_gdtr:		; void load_gdtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LGDT	[ESP+6]
		RET

_load_idtr:		; void load_idtr(int limit, int addr);
		MOV		AX,[ESP+4]		; limit
		MOV		[ESP+6],AX
		LIDT	[ESP+6]
		RET
; 操作缓存位 operate cache bit
_load_cr0:		; int load_cr0(void);
		MOV		EAX,CR0
		RET

_store_cr0:		; void store_cr0(int cr0);
		MOV		EAX,[ESP+4]
		MOV		CR0,EAX
		RET

_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI						; 
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

;Interrupts handlers
_asm_inthandler20:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler20
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_load_tr:	; void load_tr(int tr)
		LTR 	[ESP+4]		;load to TR Register
		RET

_ProcessSwitch:	; void ProcessSwitch(int eip, int cs)
		JMP		FAR [ESP+4]
		RET