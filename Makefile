PROG	= release.ihx
SRCS	= bios.c editor.c libs.c memory.c compiler.c functions.c main.c
OBJS	= $(SRCS:.c=.rel) 
LSTS	= $(SRCS:.c=.lst)
ASMS	= $(SRCS:.c=.asm)
SYMS	= $(SRCS:.c=.sym)
RELS	= $(SRCS:.c=.rel)
BKUP	= ~/Yandex.Disk

CC	= sdcc
CFLAG	= -mz80 -c
CFLA2	= -mz80 --code-loc 0x1500 --data-loc 0x3ef0 --no-std-crt0
ASM	= sdasz80

$(PROG): $(OBJS) crt/crt.rel
	$(CC) crt/crt.rel $(RELS) $(CFLA2)
	mv crt.ihx release.ihx
	cp release.ihx $(BKUP)/release.hex

all: $(PROG)

crt/crt.rel: crt/crt.asm
	$(ASM) -o crt/crt.rel crt/crt.asm

main.rel: main.c main.h macros.h
	$(CC) $(CFLAG) main.c

functions.rel: functions.c main.h macros.h
	$(CC) $(CFLAG) functions.c

compiler.rel: compiler.c main.h macros.h
	$(CC) $(CFLAG) compiler.c

memory.rel: memory.c main.h macros.h
	$(CC) $(CFLAG) memory.c

libs.rel: libs.c main.h macros.h
	$(CC) $(CFLAG) libs.c

editor.rel: editor.c main.h macros.h
	$(CC) $(CFLAG) editor.c

bios.rel: bios.c main.h macros.h
	$(CC) $(CFLAG) bios.c

clean:
	rm crt/crt.rel
	rm $(OBJS)
	rm $(LSTS) 
	rm $(ASMS)
	rm $(SYMS)
	rm release.ihx

