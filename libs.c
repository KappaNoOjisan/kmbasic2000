/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
* 	for MZ2000	*
*************************/


#include "main.h"
static FILEINFO* f=FILE_INFO;
static unsigned int size;

int divInt(int b, int a){
	return a/b;
}

int modInt(int b, int a){
	return a%b;
}
// g_tempStr and g_temp161 are used here.
char* initStr(){
	g_tempStr=allocateMemory(81);
	g_tempStr[0]=0x0D;
	g_temp161=80;
	return g_tempStr;
}
void addStr(char* str2, char* str1){
	while (str1[0]!=0x0D) str1++;
	while((str1[0]=str2[0])!=0x0D) {
		if (!g_temp161) {
			// String too long.
			str1[0]=0x0D;
			break;
		}
		g_temp161--;
		str1++;
		str2++;
	}
}
void afterStr(int* var){
	char* dest;
	if (*var==0) {
		*var=(int)g_tempStr;
	} else {
		freeMemory(g_tempStr);
		dest=(char*)(*var);
		while((dest[0]=g_tempStr[0])!=0x0D){
			dest++;
			g_tempStr++;
		}
	}
}

void listCode(unsigned int from, unsigned int to){
	unsigned int sourcePos, lineNum;
	register unsigned char i,b;
	sourcePos=g_sourceMemory;
	while (sourcePos<g_lastMemory) {
		lineNum=((unsigned int*)sourcePos)[0];
		if (from<=lineNum && lineNum<=to) {
			printUnsignedDec(lineNum);
			printChar(' ');
			i=0;
			sourcePos+=5;
			while (b=((char*)sourcePos++)[0]) {
				if (b!=0x0D) ((char*)g_strBuff)[i++]=b;
			}
			((char*)g_strBuff)[i]=0x0D;
			printStr(g_strBuff);
			newLine();
		} else {
			sourcePos+=((char*)sourcePos)[2]+5;
		}
	}
}
void deleteCode(unsigned int from, unsigned int to){
	register char *sourcePos;
	register char *moveTo;
	register char *moveFrom;
	unsigned int moveSize;
	sourcePos=(char*)g_sourceMemory;
	moveTo = NULL;
	moveFrom = NULL;
	while ((int)sourcePos<g_lastMemory) {
		if (from>*(unsigned int*)sourcePos) {
			sourcePos+=sourcePos[2]+5;
			continue;
		} else {
			moveFrom=sourcePos;
			break;
		}
	}
	if (moveFrom==NULL)
		return;
	while ((int)sourcePos<g_lastMemory) {
		if (*(unsigned int*)sourcePos<=to) {
			sourcePos+=sourcePos[2]+5;
			continue;
		} else {
			moveTo=sourcePos;
			break;
		}
	}
	if (moveTo==NULL)
		moveTo=(char*)g_lastMemory;
	// delete
	if (moveFrom==(char*)g_sourceMemory) {
		moveSize=(unsigned int)moveTo-(unsigned int)moveFrom;
		g_sourceMemory+=moveSize;
	} else {
		moveSize=(unsigned int)moveFrom-g_sourceMemory;
		moveTo-=moveSize;
		memmove(moveTo,(void*)g_sourceMemory,moveSize);
		g_sourceMemory=(unsigned int)moveTo;
	}
		
}
void printError(char type){
	register char i;
	newLine();
	switch(type){
		case ERR_NOTHIN:
			return;
		case ERR_SYNTAX:
			printStr("SYNTAX ERROR (\x0D");
			for (i=0;i<3;i++)
				if (source[i]) printChar(source[i]); else break;
			printChar(')');
			break;
		case ERR_MEMORY:
			printStr("MEMORY FULL\x0D");
			break;
		case ERR_NOLINE: 
			printStr("NO SUCH LINE\x0D");
			break;			
		case ERR_RESERV:
			printStr("NOT IMPLEMENTED YET\x0D");
			break;
		case ERR_TYPEOF:
			printStr("TYPE MISMATCH\x0D");
			break;
		default:
			printStr("ERROR \x0D");
			printUnsignedDec(type);
	}
	if (g_objPointer) {
		printStr(" IN \x0D");
		printUnsignedDec(((unsigned int*)(g_objPointer-3))[0]);
	}
	newLine();
	bell();
}

void errorAndEnd(char type) {
	printError(type);
	__asm
		JP 0x1503
	__endasm;
}

void runNext(){
	g_objPointer--;
	g_objPointer+=((unsigned char*)g_objPointer)[0]+6;
	if (g_objPointer<g_lastMemory) {
		__asm
			JP _runCode
		__endasm;
	} else {
		__asm
			JP 0x1503
		__endasm;
	}
}

void runCode(void){
	register char e;
	// Check if compiled
	if (((unsigned int*)g_objPointer)[0]==0) {
		// Compile it.
		source=(char*)(g_objPointer+2);
		object=(char*)g_nextMemory;
		e=compile();
		if (e) errorAndEnd(e);
		copyByte(0xC3); // JP XXXX
		copyInt((int)runNext);
		((unsigned int*)g_objPointer)[0]=g_nextMemory;
		g_nextMemory=(unsigned int)object;
	}
	__asm
		LD HL,#_g_objPointer
		LD E,(HL)
		INC HL
		LD H,(HL)
		LD L,E
		LD E,(HL)
		INC HL
		LD D,(HL)
		PUSH DE
		RET
	__endasm;
}

void goTo(void) __naked {
	//if (hl==0) errorAndEnd(ERR_NOLINE);
	//g_objPointer=addr;
	__asm
		ld a,h
		or a,l
		jr z,0002$
		ld (_g_objPointer),hl
		jp _runCode
	0002$:
		ld a,#(ERR_NOLINE)
		push af
		inc sp
		call _errorAndEnd
		inc sp		
		ret
	__endasm;
}

/*int getRand(){
	static long seed;
	seed=seed*1103515245+12345;
	return ((int)seed)>>1;
}*/
void getRand(void){
	__asm
		LD HL,(#_g_seed)
		LD B,#15
		
	loop:
		LD A,H
		RLA    // bit 14 of HL will be in bit 7 of A
		XOR H  // XOR of bits 15 and 14 of HL will be in bit 7 of A
		RLA    // XOR of bits 15 and 14 of HL will be in carry bit
		RLA    // XOR of bits 15 and 14 of HL will be in bit 0 of A
		LD D,A // Store XOR value in D register
		LD A,H // bit 12 of HL will be in bit 4 of A
		RRA    // bit 12 of HL will be in bit 3 of A
		XOR L  // XOR of bits 12 and 3 of HL will be in bit 3 of A
		RRA    // XOR of bits 12 and 3 of HL will be in bit 2 of A
		RRA    // XOR of bits 12 and 3 of HL will be in bit 1 of A
		RRA    // XOR of bits 12 and 3 of HL will be in bit 0 of A
		XOR D
		AND #0x01
		ADD HL,HL
		OR L
		LD L,A
		DJNZ loop
		
		LD (#_g_seed),HL
		LD A,H
		AND #0x7F
		LD D,A
		LD E,L
	__endasm;
}

void saveToTape(){
	static int i;
	static int size;
	//Set file information
	__asm
		ld b,#16
		ld hl,(_f)
		ld (hl),#2
		inc hl
	0002$:
		ld a,(de)
		cp a,#0x0d
		jr z,0003$
		inc de
		ld (hl),a
		inc hl
		dec b
		jr 0002$
	0003$:
		ld (hl),a
		inc hl
		djnz 0003$
	__endasm;
	
	if (0<(size=g_lastMemory-g_sourceMemory)) { 
		f->size=size;			// size
		f->dtadr=g_sourceMemory;        // data address
		f->exadr=0;			// execution address
		f->unlist=0;
		f->autorun=0;
		for(i=0;i<(sizeof(f->comnt));i++) f->comnt[i]=0;
	__asm
		rst 0x28
		.db 0x0e
		rst 0x28
		.db 0x0f
	__endasm;
		
	}
}

void loadFromTape(){

	__asm
		ld a,d
		or a,a
		jr nz,0007$
		ld de,#(0003$)
	0007$:	rst 0x28
		.db #0x2b
		jr c,0004$
	__endasm;

	size = f->size;
	newLine();
	__asm
		ld hl,#(0005$)
		push hl
		call _printStr
		pop af
		ld hl,(_size)
		push hl
		call _printUnsignedDec
		pop af
		ld hl,#(0006$)
		push hl
		call _printStr
		pop af	
	__endasm;
	newLine();	
	// Prepare to load
	clearMemory();
	g_sourceMemory=g_lastMemory;
	if (g_lastMemory-g_firstMemory<size) errorAndEnd(ERR_MEMORY); // Not enough memory to load
	g_sourceMemory=g_lastMemory-size;
	f->dtadr=g_sourceMemory;
	// Load data
	__asm
		rst 0x28
		.db #0x11
		jp 0x1503
	0003$:
		.db #0x0d
	0004$:
		ld a,#(ERR_NOTHIN);
		push af
		inc sp
		call _errorAndEnd
		inc sp
		ret

	0005$:
		.ascii "LOADING "
		.db 0x0d
	0006$:
		.ascii " BYTES"
		.db 0x0d
	__endasm;

}
