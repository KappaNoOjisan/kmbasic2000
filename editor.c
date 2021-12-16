/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/

#include "main.h"

#ifdef LOCAL_TEST
#else
#pragma save
#pragma disable_warning 85
void z80memcpy(char* des, char* sce, size_t size) __naked {

	__asm
	ld iy,#2
	add iy,sp

	ld e,0(iy)
	ld d,1(iy)

	ld l,2(iy)
	ld h,3(iy)

	ld c,4(iy)
	ld b,5(iy)

	ldir

	ret
	__endasm;
}
#pragma restore
#endif

void newCode(){
	// Set start point of source to last of memory
	g_sourceMemory=g_lastMemory;
	// Clear all variables
	clearMemory();
	// idTable
	clearId();
}

unsigned int getDecimal(void) __naked {

	__asm
		ld de,(_source)
		call __decI
		ld (_source),de
		ret	
	__endasm;
/*
	char b;
	int i=0;
	while(1) {
		b=source[0];
		if (b<'0' || '9'<b) break;
		source++;
		i*=10;
		i+=b-'0';
	}
	return i;
*/
}

/*	Format of a line.
+0     LSB line no
+1     MSB line no
+2     source length (n)
+3     LSB address of object code
+4     MSB address of object code
+5     source begin

+n+4   source end (must end with '0x00')

+n+5  next line
*/
INT sourceStart;
BYTE sourceLen;
INT lineNum;
BYTE lineLen;

char* findLine(void) __naked {
	__asm
	ld hl,(_g_sourceMemory)
	ld de,(_lineNum)
	ld bc,(_g_lastMemory)
00002$:
	;if hl>=_g_lastMemory break
	xor a
	sbc hl,bc
	jr nc,00003$

	add hl,bc	

	;lineLen = sourceStart[2]+5;
	inc hl
	inc hl
	ld a,(hl)
	add a,#5
	ld (_lineLen),a

	; if (hl)>=d then 
	dec hl
	ld a,(hl)
	cp d
	jr c,00004$
	
	dec hl
	ld a,(hl)
	cp e
	ret nc
00004$:
	ld a,(_lineLen)
	add a,l
	ld l,a
	jr nc,0002$
	inc h
	jr 00002$
00003$:
	; not found
	ld h,b
	ld l,c
	inc hl
	ld a,#0x7f
	inc a
	ret
	__endasm;
}
char addCode(void) __naked {
	__asm
	call _clearMemory

	; Determine line number
	call _getDecimal
	ld (_lineNum),hl

	; Determine source length and go back to original source position.
	call _skipBlank
	ld hl,(_source)

	ld e,#0
00003$:
	ld a,(hl)
	inc e
	inc hl
	or a
	jr z,00002$
	jr 00003$

00002$:
	ld a,e
	ld (_sourceLen),a
	dec e
	ld hl,(_g_sourceMemory)
	jr z,00006$
	
	; Check if available memory.

	; c=sourceLen+5
	add a,#5
	ld c,a

	; hl=g_sourceMemory-g_firstMemory
	ld de,(_g_firstMemory)
	or a
	sbc hl,de

	; if hl<bc return ERR_MEMORY
	xor a
	ld b,a
	sbc hl,bc
	jr nc,00004$

	ld l,#(ERR_MEMORY)
	ret

00004$:
	; sourceStart=g_sourceMemory
	add hl,de
	add hl,bc
00006$:
	ld (_sourceStart),hl

00005$:
	call _findLine
	ld (_sourceStart),hl
	jr nz,00008$

	; Delete the line with same number

	; bc=sourceStart-g_sourceMemory
	ld de,(_g_sourceMemory)
	xor a
	sbc hl,de
	push hl
	push af

	; de = g_sourceMemory+lineLen
	; hl = g_sourceMemory

	ld hl,(_lineLen)
	ld h,a
	add hl,de
	ld (_g_sourceMemory),hl
	ex de,hl

	pop af
	pop bc
	jr z,00008$

	add hl,bc

	ex de,hl
	add hl,bc
	ex de,hl
	dec hl
	dec de
	lddr
00008$:

	ld hl,(_g_sourceMemory)
	ld de,(_g_lastMemory)
	xor a
	sbc hl,de
	push af
	ld b,#1
	add hl,de
	pop af

	jr c,00001$

	; if hl>=de
	ld b,a
	ex de,hl
	ld (_g_sourceMemory),hl

00001$:
	; Return if source code is null (the command is "delete a line").
	ld a,(_sourceLen)
	dec a
	jr nz,00009$

	ld l,#(ERR_NOTHIN)
	ret	

00009$:
	ld d,b

	; bc=sourceLen+5
	add a,#6
	ld c,a
	xor a
	ld b,a

	dec d
	inc d	
	jr nz,00012$

	;The first line of code.

	;g_sourceMemory-=bc
	inc hl
	or a
	sbc hl,bc

	ld (_g_sourceMemory),hl
	ld (_sourceStart),hl

	jr 00010$

00012$:
	;Make a space for new line

	ld (_sourceStart),hl
	push bc
	call _findLine
	pop bc
00007$:
	;sourceStart-=bc
	push hl ; (sourceStart
	or a
	sbc hl,bc
	ld (_sourceStart),hl

	; g_sourceMemory-=bc+1
	ld hl,(_g_sourceMemory)	
	push hl ; (g_sourceMemory
	or a
	sbc hl,bc
	ld (_g_sourceMemory),hl

	;bc=sourceSrart-g_sourceMemory
	pop de ; )g_sourceMemory
	pop hl ; )sourceStart
	or a
	sbc hl,de
	jr z,00010$

	ld b,h
	ld c,l

	ld hl,(_g_sourceMemory)
	ex de,hl

	ldir

	ex de,hl


00010$:
	; Insert a line
	ld de,(_lineNum)
	ld (hl),e
	inc hl
	ld (hl),d
	inc hl
	ld a,(_sourceLen)
	ld (hl),a
	inc hl
	xor a
	ld (hl),a
	inc hl
	ld (hl),a
	inc hl
	ld (_sourceStart),hl

	; memcpy sourceStart,source,sourceLen

	ex de,hl
	ld hl,(_source)
	ld b,a
	ld a,(_sourceLen)
	ld c,a
	ldir

	ld l,#(ERR_NOTHIN)
	ret
	__endasm;
}


