;haribote-os
;TAB = 4

		ORG	0xc200				;load code on 0xc200


		MOV AL,0x13				;VGA initialization
		MOV AH,0x00
		INT 0x10

fin:
		HLT						;
		JMP		fin				;