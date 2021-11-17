/************************
*  KM-BASIC for KMZ-80  *
*       Katsumi         *
* License: LGPL ver 2.1 *
*************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "macros.h"
typedef uint8_t BYTE;
typedef uint16_t INT;
typedef char (*FUNCPTR)(void);

#ifdef LOCAL_TEST

#define __naked
typedef struct {
	FUNCPTR ptr;
	char *kw;
} STATEMENT_LIST;

typedef unsigned char *OBJECT_CODE;

#else

typedef char STATEMENT_LIST;
typedef char *OBJECT_CODE;

#endif

#define MAX_NAME_LEN 2
#define MAX_ID_COUNT 26
typedef char NAME[MAX_NAME_LEN];
typedef unsigned char TYPE;
typedef struct {
	NAME name;
	TYPE type;
} ID;

typedef struct {
	unsigned char vofs;
	unsigned int step;
	unsigned int limit;
	unsigned int jmp;
} FOR_TABLE;

typedef unsigned int SUB_TABLE;

#define LINE_BUFFER 0x1380
#define FILE_INFO 0x1300
#define FIRST_MEMORY 0x4800
#define LAST_MEMORY 0xdffe
#define MAX_SUB_COUNT 6 
#define MAX_FOR_COUNT 8
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
//
#define ID_OVER -1
#define ID_DUPL -2
#define ID_NOTF -3

// variable descriptor
#define VAR_NULL 0x00
#define VAR_USED 0x80
#define VAR_INTT 0x40
#define VAR_STRT 0x20
#define VAR_ARYT 0x10

#define TOK_NLIT 0x01
#define TOK_SLIT 0x02
#define TOK_MINS 0x03
#define TOK_LPAR 0x04
#define TOK_STRF 0x05
#define TOK_INTF 0x06
#define TOK_WHAT 0x07

// error code
#define ERR_NOTHIN 0
#define ERR_SYNTAX 1
#define ERR_MEMORY 2
#define ERR_NOLINE 3
#define ERR_TYPEOF 4
#define ERR_MISFOR 5
#define ERR_MISSUB 6
#define ERR_STKOVR 7
#define ERR_RESERV 8
#define ERR_DUPLID 9
#define ERR_SUBSCR 10
#define MAX_ERR_COUNT 11

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
	volatile const char g_variables[MAX_ID_COUNT*2];
	volatile int  g_varlimit[MAX_ID_COUNT];
	volatile const unsigned int g_firstMemory=0, g_lastMemory=0, g_nextMemory=0, g_sourceMemory=0;
	unsigned int g_objPointer, g_ifElseJump, g_seed;
	unsigned int g_temp161, g_temp162;
	char* g_tempStr;
	volatile char* source;
	char* object;
	char wkbuff[160+1];
	char countFor;
	char countSub;
#else
	extern const char g_strBuff[];
	extern const char g_variables[MAX_ID_COUNT*2];
	extern int g_varlimit[MAX_ID_COUNT];
	extern const unsigned int g_firstMemory, g_lastMemory, g_nextMemory, g_sourceMemory;
	extern unsigned int g_objPointer, g_ifElseJump, g_seed;
	extern unsigned int g_temp161, g_temp162;
	extern char* g_tempStr;
	volatile extern char* source;
	extern char* object;
	extern char countFor;
	extern char countSub;
#endif

// Macros to turn (const unsigned int) to (unsigned int)
#define g_firstMemory (((unsigned int*)(&g_firstMemory))[0])
#define g_lastMemory (((unsigned int*)(&g_lastMemory))[0])
#define g_nextMemory (((unsigned int*)(&g_nextMemory))[0])
#define g_sourceMemory (((unsigned int*)(&g_sourceMemory))[0])

// crt/crt.asm
#ifdef LOCAL_TEST
#else
extern void mul(void);
extern void z80div(void);
#endif

// memory.c
void memoryError(void);
void clearMemory(void);
char* allocateMemory(INT len);
void freeMemory(char* back);

// bios.c
void doEvents(void) __naked;
char shiftBreak(void) __naked;
char* getInt(char *source, int* result) __naked;
char* uint2dec(unsigned int value) __naked;
void printUnsignedDec(unsigned int value) __naked;
void printDec(int value) __naked;
void printStr(char* str) __naked;
void printChar(const char value) __naked;
void printHex16(unsigned int value) __naked;
void printHex8(unsigned char value) __naked;
char getLn(char *wkbuff) __naked;
char checkStack(void) __naked;
char preCheckStack(void) __naked;
char countStack(void) __naked;
char jmpErrAndEnd(void) __naked;
char callCode(int address) __naked;

// idtable.c
void clearId(void);
signed char removeId(signed char ofs);
signed char enterId(ID *id);
signed char locId(ID *id);
TYPE checkId(void);
void getId(ID *id);
char isExp(TYPE t);
char isVar(TYPE t);

// compiler.c
void copyCode(OBJECT_CODE code, int len);
void copyByte(BYTE b);
void copyInt(INT i);
char command(char* str);
char skipBlank(void);
FUNCPTR seekList(STATEMENT_LIST* slist) __naked;
char compile(void);
char compileStr(void);
char compileInt(void);
char compilePrint(void);
char checkCountFor(void);
char compileFor(void);
char compileNext(void);
char compileDim(void);

// functions.c
char funcSubStr(void);
char compileIntFunc(void);
char compileStrFunc(void);

// editor.c
void newCode(void);
char addCode(void);
unsigned int getDecimal(void) __naked;

// libs.c
char* initStr(void) __naked;
void addStr(char* str2, char* str1) __naked;
void afterStr(int* var);
void listCode(unsigned int from, unsigned int to);
void deleteCode(unsigned int from, unsigned int to);
void printError(char type);
void errorAndEnd(char type);
void runCode(void) __naked;
char goTo(void) __naked;
void getRand(void);
void saveToTape(void);
void loadFromTape(void);
