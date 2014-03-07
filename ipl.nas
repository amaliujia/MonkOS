;start os
;TAB = 4

		CYLS EQU 10
		
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
		MOV	AX,0x0820           ;set memeory address 0x0820
								;os body will be saved from ;this address
		MOV ES,AX
		MOV CH,0  				;柱面0
		MOV DH,0 				;磁头0
		MOV CL,2 				;扇区2 
readloop:
		MOV SI,0 				;记录读取失败次数
retry:
		MOV AH,0x02 			;设置读盘操作 0x02
		MOV AL,1 				;读入一个扇区
		MOV BX,0 			
		MOV DL,0x00      		;A driver
		INT 0x13 				;调用磁盘BIOS
		JNC next				;if ok, read next 
		ADD SI,1 				;si + 1 for count
		CMP SI,5 				;try 5
		JAE error    			;SI >= 5, jump tp error
		MOV AH,0x00
		MOV DL,0x00
		INT 0x13
		JMP retry

next:
		MOV AX,ES 				
		ADD AX,0x200			;内存地址后移0x200,也就是512
								;准备读入下一个sector
		MOV ES,AX 				;因为没有ADD ES,0x200,so...
		ADD CL,1
		CMP CL,18 				;totally read 18 sectors
		JBE readloop            ;read again
		MOV CL,1
		ADD DH,1
		CMP DH,2
		JB  readloop
		MOV DH,0
		ADD CH,1
		CMP CH,CYLS
		JB  readloop
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