/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/

#include "main.h"
#pragma save
#pragma disable_warning 85
void doEvents() __naked {
	// Check BREAK key
	__asm
		rst 0x28
		.db 0x0d
		jr nz,skip1
		jp 0x1503
	skip1:
		ret
	__endasm;
}

char shiftBreak() __naked {
	__asm
		rst 0x28
		.db 0x0d
		ld l,#0x00
		ret nz
		inc l
		ret
	__endasm;
}

char* getInt(char *source, int* result) __naked {
	__asm
		ld iy,#0
		add iy,sp
		ld e,2(iy)
		ld d,3(iy)
		ld a,(de)
		cp #'$'
		jr nz,nohex
		inc de
		call __hexI
		jr endhex
	nohex:
		call __decI
	endhex:
		push bc             
		ld b,5(iy)
		ld c,4(iy)
		ld a,l
		ld (bc),a
		inc bc
		ld a,h
		ld (bc),a
		pop bc
		ld 3(iy),d
		ld 2(iy),e
		ex de,hl
		ret
	__endasm;

}

char* uint2dec(unsigned int value) __naked {

	__asm
		ld iy,#0
		add iy,sp
		ld e,2(iy)
		ld d,3(iy)
		call __stade
		ret
	__endasm;

}

void printUnsignedDec(unsigned int value) __naked {
	//printStr(uint2dec(value));
	__asm
		ld iy,#0
		add iy,sp
		ld e,2(iy)
		ld d,3(iy)
		call __stade
		jp __putStr
	__endasm;
}
void printDec(int value) __naked {
	__asm
		ld iy,#0
		add iy,sp
		ld e,2(iy)
		ld d,3(iy)
		bit 7,d
		jr z,pdnrm
		push de
		ld a,#'-'
		rst 0x28
		.db 0x01
		pop de
		ld hl,#0
		or a
		sbc hl,de
		ex de,hl
	pdnrm:
		call __stade
		jp __putStr
	__endasm;
	
}
void printStr(char* str) __naked {
	__asm
		ld hl,#2
		add hl,sp
		ld a,(hl)
		inc hl
		ld h,(hl)
		ld l,a
		jp __putStr
	__endasm;
}
void printChar(const char value) __naked {
	__asm
		ld hl,#2
		add hl,sp
		ld a,(hl)
		rst 0x28
		.db 0x01
		ret
	__endasm;
}
void printHex16(unsigned int value) __naked {
	__asm
		ld hl,#2
		add hl,sp
                ld a,(hl)
		inc hl
                ld h,(hl)
		ld l,a
		rst 0x28
		.db 0x19
		ret
	__endasm;
}
void printHex8(unsigned char value) __naked  {
	__asm
		ld hl,#2
		add hl,sp
		ld a,(hl)
		rst 0x28
		.db 0x1a
		ret
	__endasm;
}
char countStack(void) __naked {
	__asm
		cp a,#(ERR_MISFOR)
		ld hl,#(_countFor)
		ld b,#(MAX_FOR_COUNT)
		jr z,0002$
		ld hl,#(_countGosub)
		ld b,#(MAX_SUB_COUNT)
	0002$:
		ld a,(hl)
		cp b
		jr c,0003$
		ld a,#(ERR_STKOVR)
		jr _jmpErrAndEnd
	0003$:
		inc (hl)
		ret
	__endasm;

}
char jmpErrAndEnd(void) __naked {
	__asm
		push af
		inc sp
		call _errorAndEnd
		inc sp
		ret
	__endasm;
}
char preCheckStack(void) __naked {
	__asm
		cp a,#(ERR_MISFOR)
		ld hl,#(_countFor)
		jr z,0002$
		ld hl,#(_countGosub)		
	0002$:
		ld c,a
		ld a,(hl)
		or a
		ret p
		ld a,c
		jr _jmpErrAndEnd
	__endasm;
}
char checkStack(void) __naked {
	__asm
		ld hl,#(_countFor)
		ld e,#0
		ld a,(hl)
		ld (hl),e
		or a
		ld a,#(ERR_MISFOR)
		jr nz,_jmpErrAndEnd
		ld hl,#(_countGosub)
		ld a,(hl)
		ld (hl),e
		or a
		ld l,e
		ret z
		ld a,#(ERR_MISSUB)
		jr _jmpErrAndEnd
	__endasm;
}
char callCode(int address) __naked {
	__asm
		ld hl,#2
		add hl,sp
		ld e,(hl)
		inc hl
		ld d,(hl)
		ld hl,#(0002$)
		push hl
		ex de,hl
		jp (hl)
	0002$:	
		jp _checkStack
	__endasm;
}

char getLn(char *wkbuff) __naked {

	__asm
		ld iy,#0
		add iy,sp
		ld e,2(iy)
		ld d,3(iy)
		rst 0x28
		.db 0x00
		ld b,#0
		ld c,#79
		ld a,(de)
		cp #0x1b
		jr z,zbrk	
		ld h,#0x0d
	zil1:
		cp h
		jr nz,zil0
	zbrk:
		xor a
		ld (de),a
		ld l,a
		ret
	zil0:
		inc de
		inc b
		ld a,b
		cp c
		ld a,(de)
		jr c,zil1
		ld l,#2
		ret
	__endasm;
}

#pragma restore

