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
		ld d,2(iy)
		ld e,3(iy)
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
		ld iy,#0
		add iy,sp
		ld l,2(iy)
		ld h,3(iy)
		jp __putStr
	__endasm;
}
void printChar(const char value) __naked {
	__asm
		ld iy,#0
		add iy,sp
		ld a,2(iy)
		rst 0x28
		.db 0x01
		ret
	__endasm;
}
void printHex16(unsigned int value) __naked {
	__asm
		ld iy,#0
		add iy,sp
                ld l,2(iy)
                ld h,3(iy)
		rst 0x28
		.db 0x19
		ret
	__endasm;
}
void printHex8(unsigned char value) __naked  {
	__asm
		ld iy,#0
		add iy,sp
		ld a,2(iy)
		rst 0x28
		.db 0x1a
		ret
	__endasm;
}

char callCode(int address) __naked {
	__asm
		ld iy,#0
		add iy,sp
		jr skip2
		jump:
		jp (HL)
		skip2:
		ld h,5(iy)
		ld l,4(iy)
		call jump
		ld l,#0
		ret
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

