/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/

/*
	Use following command to build:
	sdcc *.rel -mz80 --code-loc 0x1500 --data-loc 0x3e00 --no-std-crt0 -Wlcrt\crt.asm.o

	Note that only 56 bytes are available in data area, starting from 0x3e00
	For char[], use constant as it will be embed in code area.
*/
#define MAIN
#include "main.h"

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
	printHex16(g_lastMemory);
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
	register char* tempObject;
	while(1){
		while(1){
			source=(char*)LINE_BUFFER;
			e=inputLine();
			if (e) break;
			e=skipBlank();
			if ('0'<=e && e<='9') {
				// Editing mode
				e=addCode();
				if (e) break;
			} else {
				// Direct execution of a statement
				g_objPointer=0;
				object=tempObject=(char*)g_nextMemory;
				e=compile();
				if (e) break;
				copyByte(0xC9); // ret
				g_nextMemory=(int)object;
				e=callCode((int)tempObject);
				if (e) break;
				if (g_nextMemory==(int)object) {
					// Destroy compiled code though new object is not attached.
					// Otherwise, memory is allocated to string etc.
					// You can go back to original position using clearMemory() function.
					g_nextMemory=(int)tempObject;
				}
			}
		}
		printError(e);
	}
}
