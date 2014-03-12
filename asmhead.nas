; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		
DSKCAC	EQU		0x00100000		
DSKCAC0	EQU		0x00008000	

;BOOT_INTO
CYLS 	EQU		0x0ff0
LEDS 	EQU		0x0ff1
VMODE	EQU		0x0ff2			;color bits
SCRNX	EQU		0x0ff4			;screen x
SCRNY	EQU		0x0ff6			;screen y
VRAM	EQU		0x0ff8			;显存地址

		ORG	0xc200				;load code on 0xc200
	
		MOV AL,0x13				;VGA initialization
								;320*200*8
		MOV AH,0x00
		INT 0x10
		MOV BYTE[VMODE],8
		MOV WORD[SCRNX],320
		MOV WORD[SCRNY],200
		MOV DWORD[VRAM],0x000a0000

;BIOS and keyboard
		MOV	AH,0x02
		INT 0x16		;keyboard BIOS
		MOV [LEDS],AL

;预留下来给PIC用

		MOV		AL,0xff
		OUT		0x21,AL
		NOP					
		OUT		0xa1,AL
		;相当于io_out(0x21,0xff),io_out(0xa1,0xff)
		;意思是禁止主PIC和从PIC的中断
		CLI		;禁止CPU级别的中断

; A20
;A20Gate

		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout



;GDT问题
;32-bit breakthrough here
;the essence of transformation from 16-bit to 32-bit is to change real mode to protected mode
;In real mode circumstance, memory address can be calculated by adding base address and address offset
;In protected mode, memory address can be gotten by segment descriptor.

;以下是切换到保护模式
[INSTRSET "i486p"]				
		LGDT	[GDTR0]			
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	
		OR		EAX,0x00000001	
		MOV		CR0,EAX
		JMP		pipelineflush

pipelineflush:
		MOV		AX,1*8			
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack
;转送bootpack
		MOV		ESI,bootpack	
		MOV		EDI,BOTPAK		
		MOV		ECX,512*1024/4
		CALL	memcpy



;将启动扇区转送到0x00100000

		MOV		ESI,0x7c00		; 
		MOV		EDI,DSKCAC		;
		MOV		ECX,512/4
		CALL	memcpy


;将剩下的，也就是开始于0x000082000的内容，复制到0x00100200之后去
		MOV		ESI,DSKCAC0+512	; 
		MOV		EDI,DSKCAC+512	;
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	
		SUB		ECX,512/4		
		CALL	memcpy



; 启动bootpack

		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 
		MOV		ESI,[EBX+20]	; 
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		JNZ		waitkbdout		
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			
		RET
; memcpy

		ALIGNB	16

; 初始化0号段
GDT0:
		RESB	8				
		DW		0xffff,0x0000,0x9200,0x00cf	;可以读写的段
		DW		0xffff,0x0000,0x9a28,0x0047 ;可以执行的段

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: