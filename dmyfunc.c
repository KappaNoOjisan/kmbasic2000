
STATEMENT_LIST forDebug[] = {
	{ 
		.ptr = compilePrint,
		.kw  = "PRINT"
	},
	{
		.ptr = compileFor,
		.kw  = "FOR "
	},
	{
		.ptr = compileNext,
		.kw  = "NEXT"
	},
	{
		.ptr = NULL,
		.kw  = ""
	}
};

void mul(void) {
}
char skipBlank(void) {
	while (*source==' ') source++;
		return *source;
}


void checkCodeMemory(int len) {
}

void copyCode(OBJECT_CODE code, int len){
	int i,j;
	for (i=0;i<len;i++) {
		j = (int)code[i];
		printf("%02d:%02x\n",i,j);
	}
}

void copyByte(char b){
	printf("%02x",b);
	object+=2;
}

void copyInt(int i){
	char l,h;
	l = i % 256;
	h = i / 256;

	printf("%02x%02x",l,h);
	object++;	
}
void errorAndEnd(char e) {
	printf("Error %d\n",e);
}



STATEMENT_LIST* statementList(void) {

	return forDebug;
}

FUNCPTR seekList(STATEMENT_LIST slist[]) {
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
