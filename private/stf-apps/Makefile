AFLAGS = -mcpu=arm920t
CFLAGS = -mlittle-endian -mcpu=arm920 -Wall \
	-nostdinc -I/usr/arm-elf/arm-elf/include \
	-I/usr/arm-elf/lib/gcc-lib/arm-elf/3.2/include \
	-I../expat -I../../public -I../../../hal/public \
	-I ../../../epxa10/build/stf/private

CPP = arm-elf-cpp
CC = arm-elf-gcc
AS = arm-elf-as
AS = arm-elf-ar
LD = arm-elf-ld -N
OBJCOPY = arm-elf-objcopy

SYSLIBS = /usr/arm-elf/arm-elf/lib/libc.a \
	/usr/arm-elf/arm-elf/lib/libm.a \
	/usr/arm-elf/lib/gcc-lib/arm-elf/3.2/libgcc.a

.SUFFIXES: .c .o .S .elf .srec .xml .html .tab

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

.xml.tab:
	xmlv $< | awk -f stftotab.awk > $*.tab

.tab.html:
	awk -v test=$* -f tabtoqry.awk $< > $*.html

#
# add tests to each here...
#
TESTOBJS  = ADC.o memory.o StfEg.o atwd_pedestal.o \
		atwd_baseline.o
TABS      = ADC.tab memory.tab StfEg.tab atwd_pedestal.tab \
		atwd_baseline.tab
XMLFILES  = ADC.xml memory.xml StfEg.xml atwd_pedestal.xml \
		atwd_baseline.xml
HTMLFILES = ADC.html memory.html StfEg.html atwd_pedestal.html \
		atwd_baseline.html
HTMLDIRS  = ADC memory StfEg atwd_pedestal atwd_baseline

all: stfserv.elf menu.elf

clean:
	rm -f *.o *.i *.bin *.elf *.srec \
		run-tests.html gendir stfDirectory.[ch] \
		$(HTMLFILES) $(TABS) all.tab \
		stftcp mk-name

STFOBJS = $(TESTOBJS) ../stf/stf.o stfDirectory.o memtests.o stfUtils.o

stfserv.elf: $(STFOBJS) stfserv.o
	$(LD) -r -o stfserv.elf \
		$(STFOBJS) stfserv.o \
		../expat/xmlparse/libexpat.a \
		../../../epxa10/lib/libhal.a \
		$(SYSLIBS)

menu.elf: $(STFOBJS) menu.o
	$(LD) -r -o menu.elf \
		$(STFOBJS) menu.o \
		../../../epxa10/lib/libhal.a \
		$(SYSLIBS)

gendir: gendir.c ../../public/stf/stf.h
	gcc -o gendir -I../../public -I../../../hal/public \
		-I/usr/include/libxml2 gendir.c -Wall -g -lxml2

#genqry: genqry.c
#	gcc -o genqry -I/usr/include/libxml2 genqry.c -Wall -g -lxml2

stfDirectory.c: $(XMLFILES) gendir
	./gendir -skeleton $(XMLFILES)

stfDirectory.h: $(XMLFILES) gendir
	./gendir -skeleton $(XMLFILES)

StfEg.c: stfDirectory.h
memory.c: stfDirectory.h
ADC.c: stfDirectory.h

test-cgi: test-cgi.c
	gcc -Wall -o test-cgi test-cgi.c

stf: stf.c
	gcc -o stf -Wall -I../../include stf.c

install: html-install

#StfEg.h: StfEg.xml stfDefn2Header.xsl
#	xalan -IN StfEg.xml -OUT StfEg.h -XSL stfDefn2Header.xsl -Q

html: $(HTMLFILES) $(TABS) all.tab

html-dirs:
	(cd /var/www/stf/xml; mkdir -p $(HTMLDIRS))

html-install: run-tests.html index.html cgi-install html html-dirs
	cp *.html /var/www/stf
	cp all.tab /var/www/stf/xml

CGIS = run-test view-results view-summary validate-param
CGIHELPERS = mk-name stftcp sumtotable.awk common.sh qrytoxml.awk \
	restotab.awk tabtosum.awk tabtohtml.awk validate-qry.awk

cgi: $(CGIS) $(CGIHELPERS)

cgi-install: $(CGIS) $(CGIHELPERS)
	cp $(CGIS) /usr/lib/cgi-bin/stf
	if [[ ! -f /var/www/stf/xml/uniq.txt ]]; then echo "0" > /var/www/stf/xml/uniq.txt; fi
	cp $(CGIHELPERS) "/usr/lib/cgi-bin/stf/xml/bin"

mk-name: mk-name.c
	gcc -o mk-name -Wall mk-name.c

stftcp: stftcp.c
	gcc -o stftcp -Wall -I../../public -I../../../hal/public stftcp.c

all.tab: $(TABS)
	cat $(TABS) > all.tab

run-tests.html: run-tests.awk all.tab
	awk -f run-tests.awk all.tab > run-tests.html








