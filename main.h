/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "macros.h"

typedef char (*FUNCPTR)(void);

#ifdef LOCAL_TEST

#define __naked

typedef struct {
	FUNCPTR ptr;
	char kw[8];
} STATEMENT_LIST;


typedef char OBJECT_CODE[20];

#else

typedef char STATEMENT_LIST;

typedef char *OBJECT_CODE;

#endif



#define LINE_BUFFER 0x1380
#define FILE_INFO 0x1300
#define FIRST_MEMORY 0x4000
#define LAST_MEMORY 0xdffe
typedef struct {
	char atbr;
	char name[16];
	char fil;
	unsigned int size;
	unsigned int dtadr;
	unsigned int exadr;
	char unlist;
	char autorun;
	char comnt[102];
} FILEINFO;

// variable descriptor
#define VAR_NULL 0x00
#define VAR_USED 0x80
#define VAR_INTT 0x40
#define VAR_ARYT 0x20

// error code
#define ERR_NOTHIN 0
#define ERR_SYNTAX 1
#define ERR_MEMORY 2
#define ERR_NOLINE 3
#define ERR_RESERV 4
#define ERR_TYPEOF 5

// Global variables.
// Note that only 56 bytes are available for global variables.
// Do not initialize global variables here but do it in init() or main() function.
// "volatile const" variables will be embedded in code area after _CODE 
// so these variables would be preserved in MONITOR program,
// but the other variables will be disrupted.
#ifdef MAIN
	volatile const char g_strBuff[]=
		"0---------------1---------------2---------------3---------------"
		"4---------------5---------------6---------------7---------------";
	volatile const char g_variables[]="AABBCCDDEEFFGGHHIIJJKKLLMMNNOOPPQQRRSSTTUUVVWWXXYYZZ";
	volatile char g_vardesc[26];
	volatile int  g_varlimit[26];
	volatile const unsigned int g_firstMemory=0, g_lastMemory=0, g_nextMemory=0, g_sourceMemory=0;
	unsigned int g_objPointer, g_ifElseJump, g_seed;
	unsigned int g_temp161, g_temp162;
	char* g_tempStr;
	volatile char* source;
	char* object;
	char wkbuff[160+1];
#else
	extern const char g_strBuff[];
	extern const char g_variables[];
	extern char g_vardesc[26];
	extern int g_varlimit[26];
	extern const unsigned int g_firstMemory, g_lastMemory, g_nextMemory, g_sourceMemory;
	extern unsigned int g_objPointer, g_ifElseJump, g_seed;
	extern unsigned int g_temp161, g_temp162;
	extern char* g_tempStr;
	volatile extern char* source;
	extern char* object;
#endif

// Macros to turn (const unsigned int) to (unsigned int)
#define g_firstMemory (((unsigned int*)(&g_firstMemory))[0])
#define g_lastMemory (((unsigned int*)(&g_lastMemory))[0])
#define g_nextMemory (((unsigned int*)(&g_nextMemory))[0])
#define g_sourceMemory (((unsigned int*)(&g_sourceMemory))[0])

void mul();

// memory.c
void memoryError();
void clearMemory();
char* allocateMemory(int len);
void freeMemory(char* back);

// bios.c
void doEvents() __naked;
char shiftBreak() __naked;
char* getInt(char *source, int* result) __naked;
char* uint2dec(unsigned int value) __naked;
void printUnsignedDec(unsigned int value) __naked;
void printDec(int value) __naked;
void printStr(char* str) __naked;
void printChar(const char value) __naked;
void printHex16(unsigned int value) __naked;
void printHex8(unsigned char value) __naked;
char getLn(char *wkbuff) __naked;
char callCode(int address) __naked;

// compiler.c

void copyCode(OBJECT_CODE code, int len);
void copyByte(char b);
void copyInt(int i);
char command(char* str);
char skipBlank();
FUNCPTR seekList(STATEMENT_LIST* slist) __naked;
char compile();
char compileStr();
char compileInt();
char compilePrint();

// functions.c
char funcSubStr();
char compileIntFunc();
char compileStrFunc();

// editor.c
void newCode();
char addCode();
unsigned int getDecimal() __naked;

// libs.c
int divInt(int b, int a);
int modInt(int b, int a);
char* initStr();
void addStr(char* str2, char* str1);
void afterStr(int* var);
void listCode(unsigned int from, unsigned int to);
void deleteCode(unsigned int from, unsigned int to);
void printError(char type);
void errorAndEnd(char type);
void runCode();
void goTo(void) __naked;
void getRand();
void saveToTape();
void loadFromTape();
