#ifndef LOCAL_TEST
#include "main.h"
#endif

signed char idCount,topId,lastId;

signed char removeId(signed char ofs) {
	register ID *ptr;
	if (!idCount) return 0;

	if (ofs==lastId) {
		ptr=&idTable[ofs];
		ptr->name[0]=0;
		ptr->name[1]=0;
		ptr->type=0;
		idCount--;
		lastId--;
		return 0;
	}

	if (ofs>lastId) return -1;

	z80memcpy((char*)&idTable[ofs],(char*)&idTable[ofs+1],sizeof(ID));
	ptr=&idTable[lastId];
	ptr->name[0]=0;
	ptr->name[1]=0;
	ptr->type=0;
	idCount--;	
	lastId--;
	return 0;	
}
#ifdef LOCAL_TEST
void clearId(void){
	lastId=topId=idCount=0;
}
signed char enterId(ID *id){
	register signed char k;
	if (idCount>MAX_ID_COUNT)
		return ID_OVER;
	for (k=0;k<idCount;k++) {
		if (0==memcmp(&idTable[k],id,sizeof(ID))) {
			return ID_DUPL;
		}
	}
	z80memcpy(&idTable[idCount],id,sizeof(ID));
	lastId = idCount;	
	idCount++;
	return lastId;
}
signed char locId(ID *id) {
	register signed char j;
	for (j=0;j<idCount;j++) {
		if (0==memcmp(&idTable[j],id,sizeof(ID.name))) {
			if ((idTable[j].type&0xf0)==(*(id).type&0xf0))
				return j;
		}
	}
	return ID_NOTF;
}
void getId(ID* id) {
	register char n;
	register char* namptr;
	namptr=(*id).name;
	for (n=0;n<MAX_NAME_LEN;n++)
		*namptr++=' ';
	n=0;
	namptr=(*id).name;
	while ('A'<=*source && *source<='Z') {
		if (n<MAX_NAME_LEN) { 
			*namptr++=*source; 
			n++; 
		}
		source++;
	}
	if ( *source == '(' || *source == '$' ) source++;
}
char isExp(TYPE b) {
	return  (b==TOK_MINS||b==TOK_LPAR||b==TOK_INTF||b==TOK_NLIT||b==VAR_INTT||b==(VAR_INTT|VAR_ARYT));
}
char isVar(TYPE b) {
	return (b==VAR_INTT||b==(VAR_INTT|VAR_ARYT)||b==VAR_STRT);
}
#else
#pragma save
#pragma disable_warning 85
signed char locId(ID *id) __naked {
	__asm

	ld iy,#(_idCount)

	ld hl,#2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl) ; de=*id

; if (!idCount) return ID_NOTF;
	ld a,0(iy)
	or a
	jr nz,00007$
00005$:
	ld l,#(ID_NOTF)
	ret
00007$:
	ld c,#0
	ld hl,#_idTable
00002$:
	ld a,c
	cp a,0(iy)
	jr nc,00005$

	push de
	push hl

; name?
	ld b,#2
00003$:
	ld a,(de)
	cp a,(hl)
	jr nz,00004$
	inc hl
	inc de
	djnz 00003$

; type?
	ex de,hl
	ld a,(de)
	and a,#0xf0
	cp a,(hl)
	jr nz,00004$

	pop hl
	pop de
	ld l,c
	ret
00004$:
	pop hl
	inc hl
	inc hl
	inc hl
	pop de
	inc c
	jr 00002$
	__endasm;	
}
void clearId(void) __naked{
	__asm
	xor a
	ld hl,#_idCount
	ld (hl),a
	ld hl,#_topId
	ld (hl),a
	ld hl,#_lastId
	ld (hl),a
	ret
	__endasm;
}
signed char enterId(ID *id) __naked {
	__asm
	ld hl,#2
	add hl,sp
	ld e,(hl)
	inc hl
	ld d,(hl) ; de=id

; if (idCount>MAX_ID_COUNT) return ID_OVER;

	ld hl,#(_idCount)
	ld a,#(MAX_ID_COUNT)
	sub a,(hl)
	ld l,#(ID_OVER)
	ret m

; for(k=0;k<idCount;k++) {

	ld c,#0
	ld hl,#_idTable
	ld iy,#_idCount
00002$:
	ld a,c
	cp a,0(iy)
	jr nc,00005$

; if (0==memcmp(&idTable[k],id,sizeof(ID))) {

	push de
	push hl
	ld b,#3
00003$:
	ld a,(de)
	cp a,(hl)
	jr nz,00004$
	inc hl
	inc de
	djnz 00003$

; return ID_DUPL;

	pop de
	pop hl
	ld l,#(ID_DUPL)
	ret
00004$:
	pop hl
	inc hl
	inc hl
	inc hl

	pop de
	inc c
	jr 00002$
; }

00005$:
	ex de,hl
	ldi
	ldi
	ldi
;
	ld hl,#_idCount
	ld a,(hl)
	ld (_lastId),a
	inc (hl)
	ld l,a
	ret
	__endasm;
}
char isExp(TYPE b) __naked {
	__asm
		ld bc,#0x0601
		ld de,#(00103$)
		jr _isCmn
	00103$:
		.db #TOK_MINS
		.db #TOK_LPAR
		.db #TOK_INTF
		.db #TOK_NLIT
		.db #VAR_INTT
		.db #(VAR_INTT|VAR_ARYT)
	__endasm;
}
char isVar(TYPE b) __naked {
	__asm
		ld bc,#0x0301
		ld de,#(00103$)
		jr _isCmn
	00103$:
		.db #VAR_INTT
		.db #(VAR_INTT|VAR_ARYT)
		.db #VAR_STRT
	__endasm;
}
void isCmn(void) __naked {
	__asm
		ld hl,#2
		add hl,sp
		ld a,(hl)
		ex de,hl
	00104$:
		cp a,(hl)
		jr z,00105$
		inc hl
		djnz 00104$
		dec c
	00105$:
		ld l,c
		ret
	__endasm;
}
void getId(ID *id) __naked {
	__asm
	ld	hl,#2
	add	hl,sp
	ld	e,(hl)
	inc	hl
	ld	d,(hl)	;de=namptr;
; for(n=0;n<MAX_NAME_LEN;n++)
	ld	b,#(MAX_NAME_LEN)
	ld	c,#0x20
	ld 	h,d
	ld	l,e	;hl=namptr
00002$:
; *namptr++=' ';
	ld	(hl),c
	inc	hl
	djnz	00002$
;
	ld	hl,(_source)
	ld	c,#0x41
; while ('A'<=*source && *source<='Z') {
00003$:
	ld	a,(hl)
	sub	c
	jr	c,00004$
	cp	#26
	jr	nc,00004$
; if (n<MAX_NAME_LEN) {
	ld	a,b
	cp	#(MAX_NAME_LEN)
	jr	nc,00005$
; *namptr++=*source
	ld	a,(hl)
	ld	(de),a
	inc	de
; n++;
	inc	b
00005$:
; source++;
	inc	hl
	jr	00003$
; if ( *source=='(' || *source=='$' ) source++;
00004$:
	ld	a,(hl)
	cp	a,#0x28
	jr	z,00006$
	cp	a,#0x24
	jr	nz,00007$
00006$:
	inc	hl
00007$:
	ld	(_source),hl
	ret
	__endasm;
}
#pragma restore
#endif

TYPE checkId(void) {
	register char b;
	register char* ptr;
	switch (b=skipBlank()) {
	case ',':
		return TOK_CMMA;
	case '(':
		return TOK_LPAR;
	case '-':
		return TOK_MINS;
	case '"':
		return TOK_SLIT;
	case '$':
		return TOK_NLIT;
	default:
		if ('0'<=b && b<='9') return TOK_NLIT;
		if ('A'<=b && b<='Z') {
			ptr=source;
			while ('A'<=*ptr && *ptr<='Z')
				ptr++;
			switch (*ptr) {
			case '$':
				if (ptr-source>MAX_NAME_LEN)
					return TOK_STRF;
				return VAR_STRT;
			case '(':
				if (ptr-source>MAX_NAME_LEN)
					return TOK_INTF;
				return (VAR_INTT|VAR_ARYT);
			default:
				if (ptr-source>MAX_NAME_LEN)
					return TOK_INTF;
				return VAR_INTT;
			}
		}
	}
	return TOK_WHAT;
}	
