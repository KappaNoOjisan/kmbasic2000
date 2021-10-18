
#define def_sharp #

#define newLine()\
	 __asm\
	 	rst	0x28\
		.db	def_sharp 0x08\
	 __endasm

#define printConstChar(x)\
	__asm\
		ld	a,def_sharp x\
		rst	0x28\
		.db	def_sharp 0x01\
	__endasm

#define bell()\
	 __asm\
		rst	0x28\
		.db	def_sharp 0x17\
	 __endasm

