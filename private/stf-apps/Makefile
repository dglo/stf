.SUFFIXES: .c .o .S .elf .srec .xml .html .tab .bin

.c.o:
	$(CC) -c $(CFLAGS) $<

.S.o:
	$(CPP) $(CPPFLAGS) -o $*.i $<
	$(AS) $(AFLAGS) -o $*.o $*.i

.elf.bin:
	$(OBJCOPY) -O binary $*.elf $*.bin
	$(CPP) $(CPPFLAGS) -DBINFILE=\"$*.bin\" -o $*-raw.i $(RAWS)
	$(AS) $(AFLAGS) -o $*-raw.o $*-raw.i
	$(LD) --script=$(RAWX) -o $*-raw.elf $*-raw.o
	$(OBJCOPY) -O binary $*-raw.elf $*.bin

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
		atwd_baseline.o pmt_hv_ramp.o pmt_hv_stability.o
TABS      = ADC.tab memory.tab StfEg.tab atwd_pedestal.tab \
		atwd_baseline.tab pmt_hv_ramp.tab pmt_hv_stability.tab
XMLFILES  = ADC.xml memory.xml StfEg.xml atwd_pedestal.xml \
		atwd_baseline.xml pmt_hv_ramp.xml pmt_hv_stability.xml
HTMLFILES = ADC.html memory.html StfEg.html atwd_pedestal.html \
		atwd_baseline.html pmt_hv_ramp.html pmt_hv_stability.html
HTMLDIRS  = ADC memory StfEg atwd_pedestal atwd_baseline pmt_hv_ramp \
	        pmt_hv_stability

all: stfserv.elf menu.elf

clean:
	rm -f *.o *.i *.bin *.elf *.srec \
		run-tests.html gendir stfDirectory.[ch] \
		$(HTMLFILES) $(TABS) all.tab \
		memory.h atwd_pedestal.h adwd_baseline.h \
		StfEg.h \
		stftcp mk-name

STFOBJS = $(TESTOBJS) ../stf/stf.o stfDirectory.o memtests.o stfUtils.o
KOBJS = ../lib/crt0.o ../lib/libkernel.a

stfserv.elf: stfserv.o $(STFOBJS)
	$(LD) --script=$(KERNELX) -o stfserv.elf \
		$(KOBJS) stfserv.o $(STFOBJS) $(LIBHAL) $(LIBEXPAT) $(SYSLIBS)

menu.elf: menu.o $(STFOBJS)
	$(LD) --script=$(KERNELX) -o menu.elf \
		$(KOBJS) menu.o $(STFOBJS) $(LIBHAL) $(SYSLIBS)

$(STFSERVBINGZ): stfserv.bin
	gzip -c stfserv.bin > $(STFSERVBINGZ)

$(MENUBINGZ): menu.bin
	gzip -c menu.bin > $(MENUBINGZ)

gendir: gendir.c ../public/stf/stf.h
	gcc -o gendir -I../public \
		-I/usr/include/libxml2 gendir.c -Wall -g -lxml2

#genqry: genqry.c
#	gcc -o genqry -I/usr/include/libxml2 genqry.c -Wall -g -lxml2

stfDirectory.c: $(XMLFILES) gendir
	./gendir -skeleton $(XMLFILES)

stfDirectory.h: $(XMLFILES) gendir
	./gendir -skeleton $(XMLFILES)

StfEg.c: stfDirectory.h StfEg.h
memory.c: stfDirectory.h memory.h
ADC.c: stfDirectory.h
atwd_pedestal.c: stfDirectory.h atwd_pedestal.h
atwd_baseline.c: stfDirectory.h atwd_baseline.h
pmt_hv_ramp.c: stfDirectory.h
pmt_hv_stability.c: stfDirectory.h

memory.h:
	ln -s stfDirectory.h memory.h

atwd_pedestal.h:
	ln -s stfDirectory.h atwd_pedestal.h

atwd_baseline.h:
	ln -s stfDirectory.h atwd_baseline.h

StfEg.h:
	ln -s stfDirectory.h StfEg.h

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

