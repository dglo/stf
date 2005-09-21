AFLAGS = -mcpu=arm920t
CFLAGS = -mlittle-endian -mcpu=arm920 -Wall \
	-nostdinc -I/usr/local/arm-elf/arm-elf/include \
	-I/usr/local/arm-elf/lib/gcc-lib/arm-elf/3.2/include \
	-I../expat/xmlparse -I../../include/stf

CPP = arm-elf-cpp
CC = arm-elf-gcc
AS = arm-elf-as
LD = arm-elf-ld -N
OBJCOPY = arm-elf-objcopy

SYSLIBS = /usr/local/arm-elf/arm-elf/lib/libc.a \
	/usr/local/arm-elf/lib/gcc-lib/arm-elf/3.2/libgcc.a

.SUFFIXES: .c .o .S .elf .srec

.c.o:
	$(CC) -c $(CFLAGS) $<

.S.o:
	$(CPP) $(CPPFLAGS) -o $*.i $<
	$(AS) $(AFLAGS) -o $*.o $*.i

.elf.srec:
	$(OBJCOPY) -O binary $*.elf $*.bin
	$(CPP) $(CPPFLAGS) -DBINFILE=\"$*.bin\" -o $*-raw.i raw.S
	$(AS) $(AFLAGS) -o $*-raw.o $*-raw.i
	$(LD) --script=raw.x -o $*-raw.elf $*-raw.o
	$(OBJCOPY) -O srec $*-raw.elf $*.srec

all: stfserv.elf

clean:
	rm -f *.o *.i *.bin *.elf *.srec gendir stfDirectory.c

TESTOBJS = memory.o StfEg.o
STFOBJS = $(KOBJS) $(TESTOBJS) stfserv.o ../stf/stf.o stfDirectory.o
XMLFILES = memory.xml StfEg.xml
LINKERSCR = ../../../dom-loader/source/epxa10/loader/kernel.x

stfserv.elf: $(STFOBJS) 
	$(LD) --script=$(LINKERSCR) \
		-o stfserv.elf \
		$(STFOBJS) \
		../expat/xmlparse/libexpat.a \
		$(SYSLIBS)

stfserv.srec: stfserv.elf

gendir: gendir.c ../../include/stf/stf.h
	gcc -o gendir -I../../include/stf \
		-I/usr/include/libxml2 gendir.c -Wall -g -lxml2

stfDirectory.c: $(XMLFILES) gendir
	./gendir $(XMLFILES)

test-cgi: test-cgi.c
	gcc -Wall -o test-cgi test-cgi.c
