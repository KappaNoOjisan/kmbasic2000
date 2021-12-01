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
unsigned int sourceStart;
unsigned int lineNum,lineLen;
unsigned int sourceLen;

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
	push de
	ld de,(_lineLen)
	add hl,de
	pop de

	jr 00002$
00003$:
	; not found
	ld h,b
	ld l,c
	xor a
	dec a
	ret
	__endasm;
}
char addCode(void) __naked {
	__asm
	call _clearMemory

	; Determine line number

	call _checkId
	dec l
	ld l,#(ERR_SYNTAX)
	ret nz

	ld hl,(_source)
	ld a,(hl)
	cp #('$')
	ret z
		
	call _getDecimal
	ld (_lineNum),hl

	call _skipBlank

	; Determine source length and go back to original source position.

	ld hl,(_source)
	ld de,#0
00003$:
	ld a,(hl)
	or a
	jr z,00002$
	inc hl
	inc e
	jr 00003$

00002$:
	inc e
	ld (_sourceLen),de
	
	; Check if available memory.

	; bc=sourceLen+5
	ld b,d
	ld a,e
	add a,#5
	ld c,a

	; hl=g_sourceMemory-g_firstMemory
	ld hl,(_g_sourceMemory)
	ld de,(_g_firstMemory)
	or a
	sbc hl,de

	; if hl<bc return ERR_MEMORY
	or a
	sbc hl,bc
	jr nc,00004$

	ld l,#(ERR_MEMORY)
	ret

00004$:
	; sourceStart=g_sourceMemory
	add hl,de
	add hl,bc
	ld (_sourceStart),hl

00005$:
	; Delete the line with same number

	call _findLine
	ld (_sourceStart),hl
	jr nz,00008$

	; bc=sourceStart-g_sourceMemory
	ld de,(_g_sourceMemory)
	or a
	sbc hl,de
	ld b,h
	ld c,l

	; de = g_sourceMemory+lineLen
	ld hl,(_lineLen)
	add hl,de
	ld (_g_sourceMemory),hl
	ex de,hl

	; hl = g_sourceMemory

	ld a,b
	or c
	jr z,00008$

	dec bc
	add hl,bc

	ex de,hl
	add hl,bc
	ex de,hl

	inc bc
	lddr
00008$:
	; Return if source code is null (the command is "delete a line").
	ld a,(_sourceLen)
	sub a,#2
	jr nc,00009$

	ld l,#(ERR_NOTHIN)
	ret
00009$:
	;if g_sourceMemory==g_lastMemory
	ld hl,(_g_sourceMemory)
	ld de,(_g_lastMemory)
	or a
	sbc hl,de
	jr nz,00012$

	;The first line of code.

	;g_sourceMemory-=sourceLen+5
	ex de,hl
	ld a,(_sourceLen)
	add a,#5
	ld c,a
	xor a
	ld b,a
	sbc hl,bc
	ld (_g_sourceMemory),hl
	ld (_sourceStart),hl
	jr 00010$

00012$:

	;Make a space for new line

	;bc=sourceStart-g_sourceMemory
	call _findLine
	ld (_sourceStart),hl
	ld de,(_g_sourceMemory)
	or a
	sbc hl,de
	ld b,h
	ld c,l

	;de=g_sourceMemory-sourceLen-5
	ex de,hl
	push hl	; (g_sourceMemory

	ld a,(_sourceLen)
	add a,#5
	ld e,a
	xor a
	ld d,a

	sbc hl,de
	ld (_g_sourceMemory),hl

	push hl
	ld hl,(_sourceStart)
	or a
	sbc hl,de
	ld (_sourceStart),hl
	pop de

	pop hl ; )g_sourceMemory

	ldir

00010$:
	; Insert a line
	ld hl,(_sourceStart)
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
	ld bc,(_sourceLen)
	ldir

	ld l,#(ERR_NOTHIN)
	ret
	__endasm;
}


