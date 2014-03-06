;start os
;TAB = 4

		ORG	0x7c00		; set code loading address

		JMP	entry

;to start, firstly, we need initialize FAT12 file system.
		DB  0x90
		DB "MONKOS  "		; name of Boot
		DW	512				; size of sector
		DB	1				; size of cluster
		DW	1				; starting position of FAT
		DB	2				; size of FAT, must 2
		DW  224				;
		DW	2880			; total num osf sectors
		DB	0xf0			; type of disk
		DW	9				; length of FAT
		DW	18  			;
		DW  2
		DD	0
		DD	2880
		DB	0, 0, 0x29
		DD	0xffffffff
		DB	"MONKOS     " 	; name of disk, 11 bytes
		DB	"FAT12   "		; Format name
		RESB 18				


; body of OS
entry:
		MOV		AX,0			; init AX
		MOV		SS,AX
		MOV		SP,0x7c00		;从0x7c00开始执行
		MOV		DS,AX
;loader
		MOV	AX,0x0820
		MOV ES,AX
		MOV CH,0  				;柱面0
		MOV DH,0 				;磁头0
		MOV CL,2 				;扇区2 

		MOV SI,0 				;记录读取失败次数

retry:
		MOV AH,0x02 			;设置读盘操作 0x02
		MOV AL,1 				;读入一个扇区
		MOV BX,0 			
		MOV DL,0x00
		INT 0x13 				;调用磁盘BIOS
		JNC fin
		ADD SI,1
		CMP SI,5
		JAE error    			;SI >= 5, jump tp error
		MOV AH,0x00
		MOV DL,0x00
		INT 0x13
		JMP retry
fin:
		HLT						;
		JMP		fin				;

error:
		MOV SI,msg

putloop:
		MOV		AL,[SI]
		ADD		SI,1			; SI + 1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 
		MOV		BX,15			; 
		INT		0x10			; BIOS interrupt
		JMP		putloop

msg:
		DB		0x0a, 0x0a		; 
		DB		"hello, world"
		DB		0x0a			; 
		DB		0

		RESB	0x7dfe-$		; 

		DB		0x55, 0xaa