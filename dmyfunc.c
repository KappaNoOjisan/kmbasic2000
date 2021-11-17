STATEMENT_LIST forDebug[] = {
	{
		.ptr = compileDim,
		.kw  = "DIM"
	},{ 
		.ptr = compilePrint,
		.kw  = "PRINT"
	},{
		.ptr = compileFor,
		.kw  = "FOR "
	},{
		.ptr = compileNext,
		.kw  = "NEXT"
	},{
		.ptr = NULL,
		.kw  = ""
	}
};

void mul(void) {
}

void z80div(void) {
}

char skipBlank(void) {
	while (*source==' ') source++;
		return *source;
}


void checkCodeMemory(int len) {
}

void copyCode(OBJECT_CODE code, int len){
	int i;
	unsigned char *p=object;
	for (i=0;i<len;i++) {
		*p++=code[i];
	}
}

void copyByte(BYTE b){
	//printf("%02x",b);
	*object++=b;
}

void copyInt(INT i){
	unsigned char l,h;
	l = i % 256;
	h = i / 256;

	//printf("%02x%02x",l,h);
	*object++=l;
	*object++=h;
}
void errorAndEnd(char e) {
	printf("Error %d\n",e);
}



STATEMENT_LIST* statementList(void) {

	return forDebug;
}

char command(char* str){
	register int len;
	for(len=0;str[len];len++);
	if (strncmp(source,str,len)) return 0;
	source+=len;
	return 1;
}
FUNCPTR seekList(STATEMENT_LIST *slist) {
	while(1){
		// Fetch pointer to function
		if (slist->ptr==NULL) {
			// End of statement list (not found)
			return NULL;
		}
		// slist is now pointer to statement string to check
		if (command(slist->kw)) {
			// Statement/function found
			return slist->ptr;
		}
		slist++;
	}
		
}
void registerFor(void) {
}
char restoreFor(void) {
}
void registerSub(void) {
}
void restoreSub(void) {
}

char* getInt(char *source, int* result){
	int i;
	char b;
	if (source[0]=='$') {
		// Hexadecimal
		i=0;
		while(1){
			source++;
			b=source[0];
			if ('0'<=b && b<='9') {
				b=b-'0';
			} else if ('A'<=b && b<='F') {
				b=b-'A'+10;
			} else {
				break;;
			}
			i<<=4;
			i+=b;
		}
	} else {
		// Decimal
		for (i=0;'0'<=source[0] && source[0]<='9';source++) {
			i=i*10+(int)(source[0]-'0');
		}
	}
	result[0]=i;
	return source;
}

char* uint2dec(unsigned int value){
	unsigned char* buff=&g_strBuff[1];
	unsigned char b;
	for (b=0x30;9999<value;b++) value-=10000;
	buff[0]=b;
	for (b=0x30;999<value;b++) value-=1000;
	buff[1]=b;
	for (b=0x30;99<value;b++) value-=100;
	buff[2]=b;
	for (b=0x30;9<value;b++) value-=10;
	buff[3]=b;
	buff[4]=0x30+value;
	buff[5]=0x0D;
	for(b=0;b<4;b++){
		if (0x30<buff[b]) break;
	}
	return (char*)(&buff[b]);
}

void getRand(void){
}

void printDec(int value) {
}

char* initStr(void) {
}

char countStack(void) {
}
 
unsigned int getDecimal(){
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
}
void clearMemory(void) {
}

char goTo(void) {
}

char *allocateMemory(INT len) {
}

void saveToTape(void) {
}

void loadFromTape(void) {
}


void addStr(char* str2, char* str1) {
}

void afterStr(int* var) {
}

void listCode(unsigned int from, unsigned int to) {
}

void deleteCode(unsigned int from, unsigned int to) {
}

