/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/

#include "main.h"

void memoryError(void){
	printError(2);
	clearMemory();
	__asm
		JP 0x1503
	__endasm;
}

void clearMemory(void){
	register unsigned char b;
	register unsigned int sourceAddr;
	int* buff=(int*)&g_variables;
	for (b=0;b<26;b++) { 
		buff[b]=0x0;
		g_vardesc[b]=VAR_NULL;	// variable descriptor
		g_varlimit[b]=0x0;	// variable limit
	}
	g_nextMemory=g_firstMemory;
	// Remove pointers to each compiled routines here.
	sourceAddr=g_sourceMemory;
	while(sourceAddr<g_lastMemory){
		b=((unsigned char*)sourceAddr)[2];
		((unsigned char*)sourceAddr)[3]=0;
		((unsigned char*)sourceAddr)[4]=0;
		sourceAddr+=b+5;
	}
	// Reset random seed
	g_seed=0x3045;
}

char* allocateMemory(int len){
	char* ret=(char*)g_nextMemory;
	g_nextMemory+=len;
	if (g_sourceMemory<=g_nextMemory) memoryError();
	return ret;
}

void freeMemory(char* back){
	// Be careful and sure the allocated memories are all used temporarily.
	g_nextMemory=(unsigned int)back;
}
