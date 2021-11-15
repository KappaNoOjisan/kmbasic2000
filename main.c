/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/

#define MAIN
#include "main.h"

char *tempObject;
char *compiled;

void init(void){
	// Starting message
	newLine();
	printStr("BASIC KM-1013\x0D");
	newLine();
	// Determine the first address of available area.
	if (!g_firstMemory) {
		g_firstMemory=FIRST_MEMORY;
	}
	// Determine the last address (+1) of memory.
	g_lastMemory=LAST_MEMORY;
	printHex16(g_firstMemory);
	printConstChar(0x2D); //'-'
	printHex16(g_lastMemory+1);
	newLine();
	g_lastMemory++;
	printUnsignedDec(g_lastMemory-g_firstMemory);
	printStr(" BYTES FREE\x0D");
	newLine();
	// Destroy program and clear all variables
	newCode();
	return;
}

char inputLine(void) __naked {

	__asm
		rst 0x28
		.db #8
		ld hl,#_wkbuff
		push hl
		call _getLn
		pop af
		ld a,l
		or a
		jr z,$0004
		ret
 	$0004:
		ld de,#_wkbuff
		ld hl,(_source)
		ld b,#0x22
		ld c,#0
	$0002:
		ld a,(de)
		ld (hl),a
		or a
		jr z,$0005
		cp b
		jr nz,$0001
		dec c
		inc c
		jr nz,$0003
		inc c
		jr $0001
	$0003:
		ld (hl),#0x0d
		inc hl
		ld (hl),a
		dec c
	$0001: 
		inc de
		inc hl
		jr $0002
	$0005:
		cp c
		jr z,$0006
		ld (hl),#0x0d
		inc hl
		ld (hl),a
	$0006:
		ld l,a
		ret
	__endasm;

}
void main(void){
	register char e;
	while(1){
		while(1){
			source=(char*)LINE_BUFFER;
			e=inputLine();
			if (e) break;
			e=skipBlank();
			if ( e==0 )
				continue;
			else if ('0'<=e && e<='9') {
				// Editing mode
				e=addCode();
				if (e) break;
			} else {
				// Direct execution of a statement
				g_objPointer=0;
				tempObject=(char*)g_nextMemory;
				object=(char*)g_nextMemory;
				e=compile();
				if (e) break;
				copyInt(0x002e);// LD L,0
				copyByte(0xC9);	// RET
				compiled=object;
				g_nextMemory=(int)object;
				e=callCode((int)tempObject);
				if (e || compiled==(char*)g_nextMemory) {
					g_nextMemory=(int)tempObject;
					if (e) {
						countSub=countFor=0;
						break;
					}
				}
			}
		}
		printError(e);
	}
}
