/***
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/
#include "main.h"

FOR_TABLE forT[MAX_FOR_COUNT];
SUB_TABLE subT[MAX_SUB_COUNT];
char compileBye();
char compileClear();
char compileCursor();
char compileDelete();
char compileDim();
char compileEnd();
char compileExec();
char compileFor();
char compileGosub();
char compileGoto();
char compileLet();
char compileList();
char compileLoad();
char compileNew();
char compileNext();
char compilePoke();
char compilePrint();
char compileRet();
char compileRun();
char compileSave();

static STATEMENT_LIST slist[] = {
	{	.ptr = compileBye,
		.kw  = "BYE"
	},{
		.ptr = compileClear,
		.kw  = "CLEAR"
	},{
		.ptr = compileCursor,
		.kw  = "CURSOR"
	},{
		.ptr = compileDelete,
		.kw  = "DELETE"
	},{
		.ptr = compileDim,
		.kw  = "DIM "
	},{
		.ptr = compileEnd,
		.kw  = "END"
	},{
		.ptr = compileExec,
		.kw  = "EXEC"
	},{
		.ptr = compileFor,
		.kw  = "FOR"
	},{
		.ptr = compileGosub,
		.kw  = "GOSUB"
	},{
		.ptr = compileGoto,
		.kw  = "GOTO"
	},{
		.ptr = compileLet,
		.kw  = "LET"
	},{
		.ptr = compileList,
		.kw  = "LIST"
	},{
		.ptr = compileLoad,
		.kw  = "LOAD"
	},{
		.ptr = compileNew,
		.kw  = "NEW"
	},{
		.ptr = compileNext,
		.kw  = "NEXT"
	},{
		.ptr = compilePoke,
		.kw  = "POKE"
	},{
		.ptr = compilePrint,
		.kw  = "PRINT"
	},{
		.ptr = compileRet,
		.kw  = "RETURN"
	},{
		.ptr = compileRun,
		.kw  = "RUN"
	},{
		.ptr = compileSave,
		.kw  = "SAVE"
	}
};

#ifdef LOCAL_TEST
FUNCPTR seekList(STATEMENT_LIST slist[],unsigned char n) {
	register unsigned char i,j,k;
	unsigned char l,m;
	i=0;
	j=n;
	do {
		k=(i+j)/2;
		l=strlen(slist[k].kw);
		m=strncmp(source,slist[k].kw,l);
		if ( m<=0 ) j=k-1;
		if ( m>=0 ) i=k+1;
	} while (i<=j);
	if (i-1>j) return slist[k].ptr;
	return NULL;
}

char command(char* str){
	register int len;
	for(len=0;str[len];len++);
	if (strncmp(source,str,len)) return 0;
	source+=len;
	return 1;
}
#else
#pragma save
#pragma disable_warning 85

void z80strncmp(void) __naked {
       __asm   
               push bc
               push de
               push hl
loop:
               ld a,(de)
               sub (hl)
               jr nz, exit
               inc hl
               inc de
               djnz loop
exit:
               pop hl
               pop de
               pop bc
               ret
       __endasm;       
 
}

char command(char* str) __naked {
	__asm
	ld hl,#2
	add hl,sp
	ld a,(hl)
	inc hl
	ld h,(hl)
	ld l,a

	ld d,h
	ld e,l
	ld b,#0
00002$:
	ld a,(de)
	or a,a
	jr z,00003$
	inc de
	inc b
	jr 00002$
00003$:
	ld c,b
	ex de,hl
	ld hl,(_source)
	call _z80strncmp
	ld b,#0
	jr nz,00004$

	add hl,bc
	ld (_source),hl
	ld l,#1
	ret
00004$:
	ld l,b
	ret
	__endasm;
}

void ldForAdr(void) __naked {
	__asm
		ld a,(_countFor)
		dec a
		add a,a ;*2
		add a,a ;*4
		add a,a ;*8
		ld e,a
		ld d,#0 
		ld hl,#(_forT)
		add hl,de
		ret
	__endasm;
}
void registerFor(void) __naked {
	__asm
		push de	; limit
		push hl	; step
		push bc ; vofs
		call _ldForAdr
		ld b,#4
	00002$:
		pop de ; step..limit..jmp
		ld (hl),e
		inc hl
		ld (hl),d
		inc hl
		djnz 00002$
		push de
		ret
	__endasm;		
			
}
char restoreFor(void) __naked {
	__asm
		ld a,#(ERR_MISFOR)
		call _preCheckStack
		ret m
		;
		call _ldForAdr
		ld e,(hl)
		inc hl
		ld d,(hl) 
		inc hl
		;
		ld iy,#0
		add iy,de ; iy=varAdr
		;
		ld e,(hl)
		inc hl
		ld d,(hl) ; de=step
		inc hl
		;
		ld a,d
		ld c,(hl)
		inc hl
		ld b,(hl) ; bc=limit
		;
		push hl
		ld l,0(iy)
		ld h,1(iy)
		add hl,de
		ld 0(iy),l
		ld 1(iy),h ; (varAdr)+=step
		;
		ld e,c
		ld d,b ; de=limit
		rla
		jr nc,00002$
		ex de,hl 
	00002$:
		scf
		sbc hl,de
		pop hl
		jp p,00003$
		inc hl
		ld a,(hl)
		inc hl
		ld h,(hl)
		ld l,a
		ex (sp),hl
		ret
	00003$:
		ld hl,#(_countFor)
		dec (hl)
		ret	
	__endasm;
}
void ldSubAdr(void) __naked {
	__asm
		ld a,(_countSub)
		dec a
		add a,a
		ld e,a
		ld d,#0
		ld hl,#(_subT)
		add hl,de
		ret
	__endasm;
}
void registerSub(void) __naked {
	__asm
		ld a,#(ERR_MISSUB)
		call _countStack
		ret z
		call _ldSubAdr
		pop de ; retAdr
		ex de,hl
		ld bc,#7	; ret z; ld hl,xxxx; jp xxxx
		add hl,bc
		ex de,hl
		ld (hl),e
		inc hl
		ld (hl),d
		dec sp
		dec sp
		or a,c ; clear Z flag
		ret
	__endasm;
}
void restoreSub(void) __naked {
	__asm
		ld a,#(ERR_MISSUB)
		call _preCheckStack
		ret m
		call _ldSubAdr
		ld a,(hl) ; retAdr
		inc hl
		ld h,(hl)
		ld l,a
		ex (sp),hl
		ld hl,#(_countSub)
		dec (hl)
		ret		
	__endasm;
}
void checkCodeMemory(void) {
	//if (g_sourceMemory<=len+(int)object) memoryError();
	__asm
		push hl
		push de
		ld hl,(_object)
		add hl,bc
		ld de,(_g_sourceMemory)
		or a
		sbc hl,de
		jr c,00103$
		call _memoryError
00103$:
		pop de
		pop hl
	__endasm;
}

void copyCode(OBJECT_CODE code, int len){
	//checkCodeMemory(len+2);
	//memcpy(object,code,len);
	__asm
		ld hl,#2
		add hl,sp
		ld e,(hl)
		inc hl
		ld d,(hl)
		inc hl
		ld c,(hl)
		inc hl
		ld b,(hl)
		inc bc
		inc bc
		call _checkCodeMemory
		dec bc
		dec bc
		ld hl,(_object)
		ex de,hl
		ldir
	__endasm;
}

void copyByte(BYTE b){
	//checkCodeMemory(1);
	//*object++=b;
	__asm
		xor a
		ld b,a
		ld h,a
		inc a
		ld c,a
		ld l,a
		call _checkCodeMemory
		inc hl
		add hl,sp
		ld a,(hl)	
		ld hl,(_object)
		ld (hl),a
		inc hl
		ld (_object),hl
	__endasm;
}

void copyInt(INT i){
	//checkCodeMemory(2);
	//((int*)object)[0]=i;
	__asm
		ld bc,#2
		call _checkCodeMemory
		ld h,b
		ld l,c
		ld b,c
		add hl,sp
		ex de,hl
		ld hl,(_object)
	0002$:
		ld a,(de)
		inc de
		ld (hl),a
		inc hl
		djnz 0002$
		ld (_object),hl
	__endasm;
}
char skipBlank(void) __naked {
	__asm
		ld hl,(_source)
		ld a,#(' ')
	00102$:
		cp (hl)
		jr nz,00103$
		inc hl
		jr 00102$
	00103$:
		ld (_source),hl
		ld l,(hl)
		ret
	__endasm;
}
FUNCPTR seekList(STATEMENT_LIST slist[],unsigned char n) __naked {
	__asm
	xor	a
	ld	h,a
	ld	l,#2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
;dmyfunc.c:135: i=0;
	ld	b,a 
;dmyfunc.c:136: j=19;
	inc	hl
	ld	c,(hl)
;dmyfunc.c:137: do {
doLop:
	push	de
;dmyfunc.c:138: k=(i+j)/2;
	ld	a,b
	add	a,c
	ld	h,#0
	and	a,#0xfe
	ld	l,a
	rra
	push	af
;dmyfunc.c:139: l=strlen(slist[k].kw);
	add	hl,hl
	add	hl,de
	push	hl
	inc	hl
	inc	hl
	ld	e, (hl)
	inc	hl
	ld	d, (hl)
	push	bc
	ld	b,#0
	ld	h,d
	ld	l,e
00114$:
	ld	a,(hl)
	or	a
	jr	z,00113$
	inc	hl
	inc	b
	jr	00114$
00113$:
;dmyfunc.c:140: m=strncmp(source,slist[k].kw,l);
	ld	hl,(_source)
	ex	de,hl
	call	_z80strncmp
	jr	nz,00116$
	push	af
	ld	h,#0
	ld	l,b
	add	hl,de
	ld	(_source),hl
	pop	af
00116$:
	pop	bc
	pop	hl
;dmyfunc.c:141: if ( m<=0 ) j=k-1;
	pop	de
	jr	z,00105$
	jr	nc,00117$
00105$:
	push	af	
	ld	c,d
	dec	c
	pop	af
;dmyfunc.c:142: if ( m>=0 ) i=k+1;
00103$:
	jr	c,00115$
00117$:
	ld	b,d
	inc	b
;dmyfunc.c:143: } while (i<=j);
00115$:
	ld	a,c 
	sub	a,b
	pop	de
	jp	p,doLop
;dmyfunc.c:144: if (i-1>j) return slist[k].ptr;
	inc	a
	jp	p, 00111$
	ld	a,(hl)
	inc	hl
	ld	h, (hl)
	ld	l,a
	jr	00110$
00111$:
;dmyfunc.c:145: return NULL;
	ld	hl, #0x0000
00110$:
;dmyfunc.c:161: }
	ret

	__endasm;
}
#pragma restore
#endif
char compileStrSub(void){
	//Prepare a code to set the pointer to DE register.
	register char b;
	register char nc;
	signed char j;
	INT var,nstr;
	ID id;
	b=checkId();
	if (b==TOK_SLIT) {
		source++;
		copyByte(0x11); // LD DE,source
		copyInt((int)source);
		while(*source!='"'&&*source!='\x0') source++;
		if (*source=='"') source++;
	} else if (b==VAR_STRT) {
		nc=0;
		id.type=b;
		getId(&id);
		j=locId(&id);
		if (j==ID_NOTF) { 
			j=enterId(&id);
			nc=1;
		}
		if (j==ID_OVER) return ERR_MEMORY;
		var=(INT)&g_variables[2*j];
		if (nc) {
			// new commer
			copyByte(0xcd);	
			copyInt((INT)initStr);	// CALL initStr
			copyByte(0x22);		// LD (var),HL	
			copyInt(var);
			copyByte(0xeb);		// EX DE,HL
		} else {
			copyInt(0x5bed);	// LD DE,(var)
			copyInt(var);
		}
		if (skipBlank()=='(') {
			// A$(xx) or A$(xx,yy) (substring function)
			source++;
			b=funcSubStr();
			if (b) return b;
			if (skipBlank()!=')') return ERR_SYNTAX;
			source++;
			return ERR_NOTHIN;
		}
	} else {
		// Functions
		return compileStrFunc();
	}
	return 0;
}
char compileStr(void) {
	register char e;
	copyByte(0xcd);
	copyInt((INT)preStr);	// call preStr
	while (1) {
		e=compileStrSub();
		if (e) return e;
		copyByte(0xcd); // CALL addStr
		copyInt((INT)addStr);
		if ('+'!=skipBlank()) {
			break;
		}
		source++;
	}
	return ERR_NOTHIN;
}
void lea(void) __naked {
// b=1
//   (sp+2) X
// b=2
//   (sp+2) Y
//   (sp+4) X
// b=3
//   (sp+2) Z
//   (sp+4) Y
//   (sp+6) X
	__asm

	; de=offset
	add a,a
	ld e,a
	xor a
	ld d,a

	; addr =g_valiables[offset]
	ld hl,#(_g_variables)
	add hl,de
	ld c,(hl)
	inc hl
	ld h,(hl)
	ld l,c
	ld (addr),hl

	ld c,b
;var1
	ld iy,#(_g_varlimit)
	add iy,de

	ld l,0(iy)
	ld h,1(iy)
	ld (limitX),hl
	dec c
	jr z,00003$

;var2
	ld iy,#(_g_extlimit)
	add iy,de

	ld l,0(iy)
	dec c
	jr nz,00002$

	ld h,1(iy)	
	ld (limitY),hl
	jr lea1

;var3
00002$:
	ld h,a
	ld (limitY),hl
	
	ld l,1(iy)
	ld (limitZ),hl
	
00003$:
	push ix
	ld ix,#limitX

	; de= b*2
	ld d,a
	ld a,b
	add a,a
	ld e,a

	; iy=(sp+b*2)
	ld iy,#2
	add iy,sp
	add iy,de

	ld c,b
00004$:

	ld l,0(ix)
	ld h,1(ix) ; hl=limit

	ld e,0(iy)
	ld d,1(iy) ; de=stack

	;if offset<0 then abend;
	dec d
	inc d
	jp m,abend

	;if limit<offset then abend
	xor a
	sbc hl,de ; hl<de
	jp m,abend

	ld de,#-2
	add ix,de
	add iy,de
	djnz 00004$

	ld b,c
	pop ix

	pop hl ; retAdr
	ld (retSv),hl
lea1:
	; addr+=offsetX;

	ld hl,(addr)
	pop de
	add hl,de
	add hl,de
	ld (addr),hl
	dec c
	jr z,exitlea

lea2:
	; addr+=offsetY*(limitX+1)
	pop de

	ld hl,(limitX)
	inc hl
	push hl

	push bc
	ex de,hl
	add hl,hl
	call _mul
	pop bc

	ex de,hl
	ld hl,(addr)
	add hl,de
	ld (addr),hl

	pop hl

	dec c
	jr z,exitlea

lea3:
	; addr+=offsetZ*(limitX+1)*(limitY+1)
	pop de

	ex de,hl
	add hl,hl
	call _mul

	ld de,(limitY)
	inc de
	call _mul
	
exitlea:
	xor a
	ld iy,(retSv)
	jp (iy)

abend:
	ld l,#(ERR_SUBSCR)
	ld b,c
spret:
	pop de
	djnz spret
	ld iy,(retSv)
	jp (iy)

addr  : .dw 0
limitZ: .dw 0
limitY:	.dw 0
limitX:	.dw 0

retSv:  .dw 0

	__endasm;
}
char compileIntSub(void){
	register char b;
	int i;
	signed char j;
	ID id;
	BYTE ndim,nc;
	INT limit[MAX_DIM_COUNT];
	// Value will be in DE.
	switch (b=checkId()) {
	case TOK_LPAR:	
		source++;
		b=compileInt();
		if (b) return b;
		if (skipBlank()!=')') return ERR_SYNTAX;
		source++;
		break;
	case TOK_MINS:
		source++;
		switch (b=checkId()) {
		case TOK_NLIT:
			// Hexadecimal or decimal
			source=getInt(source,&i);
			copyByte(0x11);
			copyInt((INT)-i);
			break;
		default:
			b=compileIntSub();
			if (b) return b;
			// XOR A; LD H,A; LD L,A; SBC HL,DE; EX DE,HL
			copyCode("\xaf\x67\x6f\xed\x52\xeb",6);
			object+=6;
			break;
		}
		break;
	case TOK_NLIT:
		// Hexadecimal or decimal
		source=getInt(source,&i);
		copyByte(0x11); // LD DE,XXXX
		copyInt((INT)i);
		break;
	case (VAR_INTT|VAR_ARYT):
		getId(&id);
		id.type=b;
		j=locId(&id);
		if (j==ID_NOTF) return ERR_SUBSCR;
		ndim = idTable[j].type & 0x0f;
		nc=1;
		b=compileInt();
		if (b) return b;
		copyByte(0xD5); //PUSH DE
		while (TOK_CMMA==checkId()) {
			*source++;
			if (++nc>ndim)
				return ERR_SUBSCR;
			// extra
			b=compileInt();
			if (b) return b;
			copyByte(0xD5); // PUSH DE
		}
		if (*source!=')') return ERR_SYNTAX;
		source++;
		// LD A,#j 
		copyInt((j*0x100)+0x3e);
		// LD B,#ndim
		copyInt(ndim*0x100+0x06);
		// CALL lea; RET M
		copyByte(0xCd);
		copyInt((INT)lea);
		copyByte(0xf8);
		// LD E,(HL); INC HL; LD D,(HL)
		copyCode("\x5e\x23\x56",3);
		object+=3;
		break;
	case VAR_INTT:
		getId(&id);
		id.type=b;
		j=locId(&id);
		if (j==ID_NOTF) j=enterId(&id);
		i=(int)(&g_variables)+2*(int)j;
		// Integer variables
		copyCode("\xED\x5B",2); // LD DE,(XXXX)
		((int*)object)[1]=i;
		object+=4;
		break;
	case TOK_INTF:
		b=compileIntFunc();
		if (b) return b;
		break;
	default:
		return ERR_SYNTAX; 
	}
	return ERR_NOTHIN; 
}
char compileIntTerm(void) {
	register char b;
	register char op;
	b=compileIntSub();
	if (b) return b;
	do {	
		// Get operator
		op=skipBlank();
		switch(op){
			case '*':case '/':case '%':
				source++;
				break;
			case 'A':
				if (source[1]!='N' || source[2]!='D') return 0;
				source+=3;
				break;
			default: // Operator not found. Let's return the value (supporsed to be in DE)
				return ERR_NOTHIN;
		}
		// Preserve current DE in stack
		copyByte(0xD5); // PUSH DE
		// Get right value to DE
		b=compileIntSub();
		if (b) return b;
		// Caluculate
		switch(op){
			case '*':
				// POP HL; CALL mul
				copyCode("\xE1\xCD",2);
				object+=2;
				copyInt((int)mul);
				break;
			case '/':
				// POP HL; CALL div
				copyCode("\xE1\xCD",2);
				object+=2;
				copyInt((int)z80div);
				break;
			case '%':
				// POP HL; CALL div; EX DE,HL
				copyCode("\xE1\xCD",2);
				object+=2;
				copyInt((int)z80div);
				copyByte(0xeb);
				break;
			case 'A':
				// POP HL; LD A,H; AND D; LD H,A; LD A,L; AND E; LD L,A
				copyCode("\xE1\x7C\xA2\x67\x7D\xA3\x6F",7);
				object+=7;
				break;
			default:
				return ERR_RESERV;
		}
		copyByte(0xEB); // EX HL,DE
		// Seek the next operator
	} while(1);
}
char compileIntSxp(void) {
	register char b;
	register char op;
	// Get left value to DE
	b=compileIntTerm();
	if (b) return b;
	do {
		// Get operator
		op=skipBlank();
		switch(op){
			case '+': case '-': 
				source++;
				break;
			case 'O':
				if (source[1]!='R') return 0;
				source+=2;
				break;
			case 'X':
				if (source[1]!='O' || source[2]!='R') return 0;
				source+=3;
				break;
			default: // Operator not found. Let's return the value (supporsed to be in DE)
				return ERR_NOTHIN;
		}
		// Preserve current DE in stack
		copyByte(0xD5); // PUSH DE
		// Get right value to DE
		b=compileIntTerm();
		if (b) return b;
		// Caluculate
		switch(op){
			case '+':
				// POP HL; ADD HL,DE
				copyCode("\xE1\x19",2);
				object+=2;
				break;
			case '-':
				// POP HL; XOR A; SBC HL,DE
				copyCode("\xE1\xAF\xED\x52",4);
				object+=4;
				break;
			case 'O':
				// POP HL; LD A,H; OR D; LD H,A; LD A,L; OR E; LD L,A
				copyCode("\xE1\x7C\xB2\x67\x7D\xB3\x6F",7); 
				object+=7;
				break;
			case 'X':
				// POP HL; LD A,H; XOR D; LD H,A; LD A,L; XOR E; LD L,A
				copyCode("\xE1\x7C\xAA\x67\x7D\xAB\x6F",7);
				object+=7;
				break;
			default:
				return ERR_RESERV;
		}
		copyByte(0xEB); // EX HL,DE
		// Seek the next operator
	} while(1);
}
char compileInt(void){
	register char b;
	register char op;
	b=compileIntSxp();
	if (b) return b;
	do {
		op=skipBlank();
		switch(op){
			case '=':
				source++;
				break;
			case '!':
				if (source[1]!='=') return ERR_NOTHIN;
				source+=2;
				break;
			case '<':
				source++;
				if (source[0]=='=') {
					source++;
					op='(';
				}
				break;
			case '>':
				source++;
				if (source[0]=='=') {
					source++;
					op=')';
				}
				break;
			default: // Operator not found. Let's return the value (supporsed to be in DE)
				return ERR_NOTHIN;
		}
		// Preserve current DE in stack
		copyByte(0xD5); // PUSH DE
		// Get right value to DE
		b=compileIntSxp();
		if (b) return b;
		// Caluculate
		switch(op){
			case '=':
				// POP HL; XOR A; SBC HL,DE; LD H,A; LD L,A; JR NZ,skip:; inc HL; skip:
				copyCode("\xE1\xAF\xED\x52\x67\x6F\x20\x01\x23",9);
				object+=9;
				break;
			case '!':
				// POP HL; XOR A; SBC HL,DE; LD H,A; LD L,A; JRNZ,skip:; inc HL; skip:
				copyCode("\xE1\xAF\xED\x52\x67\x6F\x28\x01\x23",9);
				object+=9;
				break;
			case '<':
				// POP HL; XOR A; SBC HL,DE; LD H,A; LD L,A; JP P,skip:; INC HL; skip:
				copyCode("\xE1\xAF\xED\x52\x67\x6F\xF2\x0A\x00\x23",10);
				object+=7;
				((int*)object)[0]=(int)object+3;
				object+=3;
				break;
			case '>':
				// POP HL; XOR A; SBC HL,DE; LD H,A; LD L,A; JP M,skip:; JR Z,skip:; INC HL; skip:
				copyCode("\xE1\xAF\xED\x52\x67\x6F\xFA\x0C\x00\x28\x01\x23",12);
				object+=7;
				((int*)object)[0]=(int)object+5;
				object+=5;
				break;
			case '(':
				// POP HL; XOR A; SBC HL,DE; LD H,A; LD L,A; JR Z,skip1: JP P,skip:2; skip1: INC HL; skip:2
				copyCode("\xE1\xAF\xED\x52\x67\x6F\x28\x03\xF2\x0A\x00\x23",12);
				object+=9;
				((int*)object)[0]=(int)object+3;
				object+=3;
				break;
			case ')':
				// POP HL; XOR A; SBC HL,DE; LD H,A; LD L,A; JP M,skip:; INC HL; skip:
				copyCode("\xE1\xAF\xED\x52\x67\x6F\xFA\x0A\x00\x23",10);
				object+=7;
				((int*)object)[0]=(int)object+3;
				object+=3;
				break;
			default:
				return ERR_RESERV;
		}
		copyByte(0xEB); // EX HL,DE
		// Seek the next operator
	} while(1);
}
char compilePrint(void){
	register char b;
	char cr=1;
	do {
		switch ( skipBlank() ) {
			case ':':
			case 0x00:
				break;
			case ';':
				cr=0;
				source++;
				continue;
			case ',':
				cr=0;
				source++;
				// LD A,11h; RST 28H; DEFB 01
				copyCode("\x3e\x11\xef\x01",4);
				object+=4;
				continue;
			case 'E':
				if (command("ELSE ")) break;
			default:
				cr=1;
				b=checkId();
				if (b==VAR_STRT||b==TOK_STRF||b==TOK_SLIT) {
					// String
					b=compileStr();
					if (b) return b;
					copyByte(0x11);			// LD DE,wkbuff
					copyInt((INT)wkbuff);
					copyCode("\xCD\x09\x15",3);	// CALL 0x1509
					object+=3;
				} else if (isExp(b)) {
					// Integer
					b=compileInt();
					if (b) return b;
					copyCode("\xD5\xCD\x00\x00\xD1",5); // PUSH DE; CALL printDec; POP DE
					((int*)object)[1]=(int)printDec;
					object+=5;
				} else return ERR_SYNTAX;
				continue;
		}
		if (cr) {
			copyCode("\xEF\x8",2); // RST 28H; DEFB 8H
			object+=2;
		}
		return ERR_NOTHIN;
	} while (1);
}
char compileBye(){
	copyCode(
		"\xC3\xf0\x00" // JP 0x00f0
		,3);
	object+=3;
	return ERR_NOTHIN;
}
char compileEnd(){
	copyInt(0x002e);	// LD L,#0
	copyByte(0xC9);		// RET
	return ERR_NOTHIN;
}
char compileNew(){
	copyCode(
		"\xC3\x00\x15" // JP 0x1500
		,3);
	object+=3;
	return ERR_NOTHIN;
}

char compileLet(void){
	register char b,e;
	register int dest;
	signed char j;
	char ndim;
	ID id;
	char nc;
	// Seek the name of variable (either A, B, ... or Z)
	b=checkId();
	if (!isVar(b)) return ERR_SYNTAX;
	getId(&id);
	id.type=b;
	j=locId(&id);
	if (b==(VAR_INTT|VAR_ARYT)) {
		if (j==ID_NOTF) return ERR_SUBSCR;
		// Dimension
		ndim = idTable[j].type & 0x0f;
		nc=0;
		e=compileInt();
		if (e) return e;
		copyByte(0xd5); //PUSH DE
		nc=nc+1;
		while (TOK_CMMA==checkId()) {
			source++;
			if (++nc>MAX_DIM_COUNT)
				return ERR_SUBSCR;
			e=compileInt();
			if (e) return e;
			copyByte(0xD5); // PUSH DE
		}
		if (')' != *source) return ERR_SYNTAX;
		source++;
		// push effective address
		copyInt((ndim*0x100)+0x06);	// LD B,#ndim
		copyInt((j*0x100)+0x3e);	// LD A,#j
		copyByte(0xCd); 		// CALL _lea
		copyInt((INT)lea);
		copyByte(0xf8);			// RET M
		copyByte(0xe5); 		// PUSH HL
	}
	if (skipBlank()!='=') return ERR_SYNTAX;
	source++;
	// get address
	nc=0;
	if (j==ID_NOTF) {
		j=enterId(&id);
		nc=1;
	}
	if (j==ID_OVER) return ERR_MEMORY;
	dest=(int)&g_variables[j*2];
	switch (b) {
	case VAR_STRT:
		if (nc) {
			copyByte(0xcd);	
			copyInt((INT)initStr);	// CALL initStr
			copyByte(0x22);		// LD (dest),HL	
			copyInt(dest);
		} else {
			copyByte(0x2a);
			copyInt((INT)dest);	// LD HL,(dest)
		}
		copyByte(0xe5);	// PUSH HL
		e=compileStr();
		if (e) return e;
		copyByte(0xe1); // POP HL
		copyByte(0xcd);
		copyInt((INT)afterStr);	// CALL afterStr
		break;
	case VAR_INTT: case (VAR_INTT|VAR_ARYT):
		e=compileInt();
		if (e) return e;
		if (b==(VAR_INTT|VAR_ARYT)) {
			// POP HL; LD (HL),E; INC HL; LD (HL),D
			copyCode("\xe1\x73\x23\x72",4);
			object+=4;
		} else {
			copyCode("\xED\x53",2); // LD (dest),DE
			((int*)object)[1]=dest;
			object+=4;
		}
		break;
	}
	return ERR_NOTHIN;
}

void setLineNum(void){
	// Set g_objPointer
	copyCode("\x21\x34\x12\x22",4);
	((unsigned int*)object)[2]=(unsigned int)(&g_objPointer);
	object++;
	((unsigned int*)object)[0]=g_objPointer;
	object+=5;
}
/*	FOR var=initValue TO limitValue STEP stepValue 

	LD A,#ERR_MISSFOR
	CALL countStack

	; for var=init
	(LD DE,init)
	LD (var),DE
	LD BC,&var

	; TO limit
	(LD DE,limit)
	EX DE,HL

	; STEP step
	(LD DE,step)
	EX DE,HL

	CALL saveStack
*/
char compileFor(void){
	register char e;
	register signed char vofs;
	register INT vadr;
	char *idptr;
	ID id;

	copyByte(0x3e);			// LD A,ERR_MISFOR
	copyByte(ERR_MISFOR);
	copyByte(0xcd);
	copyInt((int)countStack);	// CALL countStack
	copyByte(0xC8);			// RET Z
	// init statement
	e=checkId();
	idptr=source;
	getId(&id);
	id.type=e;
	vofs=locId(&id);
	switch (e) {
	case VAR_INTT:
		if (vofs==ID_NOTF) vofs=enterId(&id);
		if (vofs==ID_OVER) return ERR_MEMORY;
		vadr=(INT)&g_variables[vofs*2];
		break;	
	case VAR_INTT|VAR_ARYT:
		if (vofs==ID_NOTF) return ERR_SUBSCR;
		vadr=(INT)&g_variables[vofs*2];
		vadr=*(INT*)vadr;	
		break;
	default:
		return ERR_SYNTAX;
	}
	source=idptr;
	e=compileLet();
	copyByte(0x01); // LD BC,#vadr
	copyInt(vadr);
	if (e) return e;
	// TO statement
	if (command("TO ")) return ERR_SYNTAX;
	source+=3;
	e=compileInt();
	if (e) return e;
	// STEP statement
	skipBlank();
	if (!command("STEP ")) {
		source+=5;
		copyByte(0xEb); // EX DE,HL
		e=compileInt();
		if (e) return e;
		copyByte(0xEb);	// EX DE,HL
	} else {
		// STEP 1
		copyByte(0x21); // LD HL,1
		copyInt(1);
	}

	copyByte(0xcd);		// CALL registerFor
	copyInt((int)registerFor);

	// g_objPointer will back to current line.
	setLineNum();
	return ERR_NOTHIN;
}
char compileNext(void){
	copyByte(0xCD);		// CALL restoreFor
	copyInt((int)restoreFor);
	copyByte(0xF8);		// RET M
	return ERR_NOTHIN;
}
/*
char compileDebug(){
	int objaddr;
	// Print object address
	objaddr=(int)object;
	copyCode("\x11\x00\x00\xCD\x15\x00",6); // LD DE,XXXX; CALL 0x0015
	object++;
	((int*)object)[0]=(int)"OBJECTS:\x0D";
	object+=5;
	copyCode("\x21\x00\x00\xCD\xBA\x03\xCD\x09\x00",9); // LD HL,XXXX; CALL 0x03BA; CALL 0x0009
	object++;
	((int*)object)[0]=objaddr;
	object+=8;
	// Print g_variables
	copyCode("\x11\x00\x00\xCD\x15\x00",6); // LD DE,XXXX; CALL 0x0015
	object++;
	((int*)object)[0]=(int)"VARIABLES:\x0D";
	object+=5;
	copyCode("\x21\x00\x00\xCD\xBA\x03\xCD\x09\x00",9); // LD HL,XXXX; CALL 0x03BA; CALL 0x0009
	object++;
	((int*)object)[0]=(int)(&g_variables[0]);
	object+=8;
	// Print stack pointer
	copyCode("\x11\x00\x00\xCD\x15\x00",6); // LD DE,XXXX; CALL 0x0015
	object++;
	((int*)object)[0]=(int)"STACK:\x0D";
	object+=5;
	copyCode("\xED\x73\x34\x12\x2A\x34\x12\xCD\xBA\x03\xCD\x09\x00",13); // LD (XXXX),SP; LD HL,(XXXX); CALL 0x03BA; CALL 0x0009
	((int*)object)[1]=(int)(&g_temp161);
	object++;
	((int*)object)[2]=(int)(&g_temp161);
	object+=12;
	return 0;
}
*/
char compileList(void){
	register char b;
	unsigned int from,to;
	from=0;
	to=65535;
	b=skipBlank();
	if ('0'<=b && b<='9') from=to=getDecimal();
	b=skipBlank();
	if (b=='-'||b==',') {
		source++;
		b=skipBlank();
		if ('0'<=b && b<='9') to=getDecimal();
		else to=65535;
	}
	copyCode("\x21\x34\x12\xE5\x21\x34\x12\xE5\xCD\x34\x12\xE1\xE1",13); // LD HL,XXXX; PUSH HL; LD HL,XXXX; PUSH HL; CALL XXXX; POP HL; POP HL;
	object++;
	((unsigned int*)object)[0]=to;
	((unsigned int*)object)[2]=from;
	((unsigned int*)object)[4]=(unsigned int)listCode;
	object+=12;
	return ERR_NOTHIN;
}
char compileClear(void){
	copyByte(0xCD);
	copyInt((int)clearMemory);
	return ERR_NOTHIN;
}
unsigned int getDest(void) {
	register unsigned int i,sourcePos;
	char b;
	b=skipBlank();
	if (b<'0' || '9'<b) {
		i=0;
	} else {
		i=getDecimal();
	}
	sourcePos=g_sourceMemory;
	while (sourcePos<g_lastMemory) {
		if (i==((unsigned int*)sourcePos)[0]) {
			sourcePos+=3;
			break;
		}
		sourcePos+=((unsigned char*)sourcePos)[2]+5;
	}
	if (g_lastMemory<=sourcePos) sourcePos=0;
	return sourcePos;
}
void genGoto(unsigned int Dest) {
	copyByte(0x21);
	copyInt(Dest);			// LD HL,Dest
	copyByte(0xC3);
	copyInt((unsigned int)goTo);	// JP goTo
}
char compileGoto(void){
	register unsigned int sourcePos;
	//check Destination
	sourcePos=getDest();
	genGoto(sourcePos);
	return ERR_NOTHIN;
}
char compileGosub(void){
	register unsigned int sourcePos;
	sourcePos=getDest();
	if (sourcePos) {
		copyByte(0xCd);			// CALL registerSub
		copyInt((int)registerSub);	
		copyByte(0xc8); 		// RET Z
	}
	genGoto(sourcePos);
	// g_objPointer will back to current line.
	setLineNum();
	return ERR_NOTHIN;
}
char compileRet(void){
	copyByte(0xcd); // CALL restoreSub
	copyInt((int)restoreSub);
	copyByte(0xF8); // RET M
	return ERR_NOTHIN;
}
char compileRun(void){
	register char b;
	copyByte(0xCD); // CALL clearMemory
	copyInt((INT)clearMemory);
	copyByte(0xCD);
	copyInt((INT)clearId); // CALL clearId
	// Inhibit compileGoto() when only "RUN" entered
	b=skipBlank();
	if (b<'0' || '9'<b) {
		// No Code, No Error
		if (g_sourceMemory==g_lastMemory) return ERR_NOTHIN;
		// goTo(g_sourceMemory+3);
		copyByte(0x21);		// LD HL,g_sourceMemory+3
		copyInt((int)g_sourceMemory+3);
		copyByte(0xC3);		// JP goTo;
		copyInt((int)goTo);
		return ERR_NOTHIN;
	} else 
		return compileGoto();
}
char compileIf(void){
	register char e;
	e=compileInt();
	if (e) return e;
	skipBlank();
	if (command("THEN ")) return ERR_SYNTAX;
	source+=5;
	copyCode("\x7A\xB3\x20\x03\xC3",5);
	object+=7;
	g_ifElseJump=(unsigned int)object-2;
	return ERR_NOTHIN;
}
char compileElse(void){
	if (!g_ifElseJump) return 1;
	((unsigned int*)g_ifElseJump)[0]=(unsigned int)object+3;
	copyByte(0xC3);
	g_ifElseJump=(unsigned int)object;
	object+=2;
	return ERR_NOTHIN;
}
char compilePoke(void){
	char e;
	e=compileInt();
	if (e) return e;
	if (skipBlank()!=',') return ERR_SYNTAX;
	source++;
	copyByte(0xD5); // PUSH DE
	e=compileInt();
	if (e) return e;
	copyCode("\xE1\x73",2); // POP HL; LD (HL),E
	object+=2;
	return ERR_NOTHIN;
}
char compileCursor(void){
	char e;
	e=compileInt();
	if (e) return e;
	// LD HL,WIDTH; LD A,E; CP (HL); LD L,3H; JR NC,skip; LD (HL),A; skip: 
	copyCode("\x21\x13\x00\x7b\xbe\x2e\x03\x30\x01\x77",10);
	object+=10;
	if (skipBlank()!=',') return ERR_SYNTAX;
	source++;
	e=compileInt();
	if (e) return e;
	// LD A,E; CP 0x19; JR NC,skip; LD (CURY),A; skip: 
	copyCode("\x7B\xFE\x19\x30\x03\x32\x04\x00",8);
	object+=8;
	return ERR_NOTHIN;
}
char allocateDim(void) __naked {
	// a : idofs
        // b : countDim 3 2 1
	//(sp+0): retA
	//(sp+2): limit z y x
	//(sp+4): limit y x
	//(sp+6): limit x
	__asm

		ld iy,#2
		add iy,sp

		ld c,b

		; retA
		pop hl
		ld (00010$),hl

		ld hl,#1

	00002$:
		pop de	; de=(sp); sp+=2

		inc d
		dec d
		jp m,00003$


		push af ; idofs(
		push bc ; countDim(
		push de ; limit(
		call _mul ; hl=hl*(limit+1)
		pop de  ; )limit
		pop bc  ; )countDim
		pop af  ; )idofs

		djnz 00002$	
		
	00004$:
		push bc ; (countDim
		push de ; (limit x
		push af ; (idofs

		inc hl
		add hl,hl ; hl=memsize
		push hl
		call _allocateMemory
		pop af
		ex de,hl  ; de=allocptr

		pop af    ; )idofs
		add a,a
		ld c,a
		ld b,#0

		ld hl,#(_g_variables)
		add hl,bc ; varptr
		ld (hl),e
		inc hl 
		ld (hl),d

		pop de ; )limitx

		ld hl,#(_g_varlimit)
		add hl,bc
		ld (hl),e
		inc hl
		ld (hl),d

		ld hl,#(_g_extlimit)
		add hl,bc

		pop bc   ; )countDim
		dec c
		jr z,00005$ ; =1
		
		ld e,0(iy)
		ld d,1(iy)
		ld (hl),e   ;limitY
		inc hl
		ld (hl),d
		
		dec c
		jr z,00005$ ; =2	

		ld e,2(iy)
		ld (hl),e   ;limitZ
	00005$:
		xor a
	00006$:
		ld hl,(00010$)
		jp (hl)
	; error
	00003$:
		ld l,#(ERR_SUBSCR)
		dec b
		jr nz,00007$
		jp (ix)
	00007$:
		pop de
		djnz 00007$
		jr 00006$

	00010$:
		.dw 2

	__endasm;
}
char compileDim(void){
	char b;
	int j;
	ID id;

	countDim=1;
	while(1){
		b=checkId();
		if (b!=(VAR_INTT|VAR_ARYT)) return ERR_SYNTAX;
		getId(&id);
		id.type=b;
		j=locId(&id);
		if (j!=ID_NOTF) return ERR_DUPLID;

		b=compileInt();
		if (b) return b;
		copyByte(0xD5); // PUSH DE

		while(TOK_CMMA==checkId()) {
			source++;	
			if (++countDim>MAX_DIM_COUNT) {
				return ERR_SUBSCR;
			}
			// subscript
			b=compileInt();
			if (b) return b;
			copyByte(0xD5); // PUSH DE
		}
		if (*source!=')') return ERR_SYNTAX;
		source++;
		// enter
		id.type |= countDim;
		j=enterId(&id);
		if ( j==ID_OVER ) return ERR_MEMORY;

		// LD A,#j; 
		copyInt((j*0x100)+0x3e);
		// LD B,#countDim;
		copyInt(countDim*0x100+0x06);
		// CALL allocateDim; RET M;
		copyByte(0xCD);
		copyInt((INT)allocateDim);
		copyByte(0xF8);
		if (skipBlank()!=',') return ERR_NOTHIN;
		source++;
	}
}
char compileExec(void){
	char b1,b2;
	while(1){
		b1=skipBlank();
		if ('0'<=b1 && b1<='9') {
			b1=b1-'0';
		} else if ('A'<=b1 && b1<='F') {
			b1=b1-'A'+10;
		} else {
			return ERR_NOTHIN;
		}
		b1<<=4;
		b2=source[1];
		if ('0'<=b2 && b2<='9') {
			b2=b2-'0';
		} else if ('A'<=b2 && b2<='F') {
			b2=b2-'A'+10;
		} else {
			return ERR_NOTHIN;
		}
		copyByte(b1+b2);
		source+=2;
	}
}
char compileSave(void){
	char b;
	char e;
	b=skipBlank();
	e=compileStrSub();
	if (e) return e;
	copyByte(0xCD);
	copyInt((int)saveToTape);
	return ERR_NOTHIN;
}
char compileLoad(void){
	register char b;
	register char e;
	b=skipBlank();
	if (b=='"') {
		e=compileStrSub();
		if (e) return e;
	} else {
		//LD D,0;
		copyCode("\x16\00",2);
		object+=2;
		
	}
	copyByte(0xCD);
	copyInt((int)loadFromTape);
	return ERR_NOTHIN;
}
char compileDelete(void){
	register char b;
	unsigned int from,to,tmp;
	from=0;
	to=65535;
	b=skipBlank();
	if (b==0)
		return ERR_NOLINE;
	else if ('0'<=b && b<='9') {
		from=getDecimal();
		to=from;
		b=skipBlank();
	}
	if (b=='-'||b==',') {
		source++;
		b=skipBlank();
		if ('0'<=b && b<='9') {
			tmp=getDecimal();
			if (tmp>from) to=tmp; else from=tmp;
		}
	}
	copyCode("\x21\x34\x12\xE5\x21\x34\x12\xE5\xCD\x34\x12\xE1\xE1",13); // LD HL,XXXX; PUSH HL; LD HL,XXXX; PUSH HL; CALL XXXX; POP HL; POP HL;
	object++;
	((unsigned int*)object)[0]=to;
	((unsigned int*)object)[2]=from;
	((unsigned int*)object)[4]=(unsigned int)deleteCode;
	object+=12;
	return ERR_NOTHIN;
}
char compile(void) {
	FUNCPTR sfunc;
	register char e=ERR_NOTHIN;
	g_ifElseJump=0;
	while (skipBlank()!=0x00) {
		if (command("IF ")) {
			e=compileIf();
			continue;
		} else if (command("REM")) {
			// Skip until 0x00
			while(source[0]) source++;
			break;
		} else {
			sfunc=(FUNCPTR)seekList(slist,sizeof(slist)/sizeof(slist[0])-1);
			if (sfunc) {
				// Statement found
				e=sfunc();
			} else {
				// Statement not found
				// Must be LET statement
				e=compileLet();
			}
		}
		if (e) return e;
		e=skipBlank();
		if (e==':') {
			source++;
			continue;
		} else if (e==0x00) {
			break;
		} else if (command("ELSE ")) {
			e=compileElse();
			if (e) return e;
		} else {
			return ERR_SYNTAX;
		}
	}
	if (g_ifElseJump) ((unsigned int*)g_ifElseJump)[0]=(unsigned int)object;
	return ERR_NOTHIN;
}
