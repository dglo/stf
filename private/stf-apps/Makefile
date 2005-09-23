#
# adding a new test?  add the .o to TESTOBJS below...
#
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

#
# add new tests here...
#
TESTOBJS  = ADC.o memory.o StfEg.o atwd_pedestal.o atwd_baseline.o \
	pmt_hv_ramp.o pmt_hv_stability.o pressure.o pulser_spe_mpe.o \
	atwd_pedestal_sweep_forced.o atwd_pulser_spe.o atwd_pedestal_spe.o \
	atwd_clock1x_forced.o flasher_brightness.o flasher_width.o \
	fadc_fe_pulser.o fadc_baseline.o fadc_fe_forced.o disc_scan.o \
	loopback_lc.o atwd_pedestal_noise.o temperature.o atwd_pmt_spe.o \
	fadc_fe_external.o atwd_pmt_led.o atwd_clock2x_forced.o \
	flasher_clock.o atwd_led_nopmt.o

XMLFILES  = $(TESTOBJS:.o=.xml)
CFILES    = $(TESTOBJS:.o=.c)
XMLTEMPLATES = $(TESTOBJS:%.o=%-template.xml)

$(XMLTEMPLATES): %-template.xml: %.xml
	xmln $< | awk -f ../stf-sfe/stftotab.awk | awk -f mktemplate.awk > $@

#
# generated links...
#
HDRFILES  = $(TESTOBJS:.o=.h)

#
# java code gen...
#
GENJAVA=../../../stf/src/icecube/daq/stf/autogen/STFParameterLookup.java

all: stfserv.elf menu.elf echomode.elf $(GENJAVA) wiggle.elf

java: $(GENJAVA)

clean:
	rm -f *.o *.i *.bin *.elf *.srec \
		gendir stfDirectory.[ch] \
		memory.h atwd_pedestal.h adwd_baseline.h \
		StfEg.h $(HDRFILES)

EXOBJS = atwdUtils.o
STFOBJS = $(TESTOBJS) ../stf/stf.o stfDirectory.o memtests.o stfUtils.o
KOBJS = ../lib/crt0.o ../lib/libkernel.a

stfserv.elf: stfserv.o $(EXOBJS) $(STFOBJS) $(LIBHAL)
	$(LD) --script=$(KERNELX) -o stfserv.elf \
		$(KOBJS) stfserv.o $(STFOBJS) $(EXOBJS) \
			$(LIBHAL) $(LIBEXPAT) $(SYSLIBS)

menu.elf: menu.o $(EXOBJS) $(STFOBJS) $(LIBHAL)
	$(LD) --script=$(KERNELX) -o menu.elf \
		$(KOBJS) menu.o $(STFOBJS) $(EXOBJS) $(LIBHAL) $(SYSLIBS)

echomode.elf: echomode.o $(LIBHAL)
	$(LD) --script=$(KERNELX) -o echomode.elf \
		$(KOBJS) echomode.o $(LIBHAL) $(SYSLIBS)

wiggle.elf: wiggle.o $(LIBHAL)
	$(LD) --script=$(KERNELX) -o wiggle.elf \
		$(KOBJS) wiggle.o $(LIBHAL) $(SYSLIBS)

../bin/%.bin.gz: %.bin; gzip -c $< > $@

gendir: gendir.c ../public/stf/stf.h
	gcc -o gendir -I../public \
		-I/usr/include/libxml2 gendir.c -Wall -g -lxml2

stfDirectory.h stfDirectory.c: $(XMLFILES) gendir Makefile
	./gendir -skeleton $(XMLFILES)

$(CFILES): %.c: %.h

$(HDRFILES): stfDirectory.h
	if [[ ! -h $@ ]]; then ln -s stfDirectory.h $@; fi

../bin/stfserv: stfserv.o $(STFOBJS) $(EXOBJS)
	gcc -o stfserv stfserv.o $(STFOBJS) $(EXOBJS) $(LIBHAL) -lm -lexpat

../bin/menu: menu.o $(STFOBJS) $(EXOBJS)
	gcc -o menu menu.o $(STFOBJS) $(EXOBJS) $(LIBHAL) -lm

../bin/echomode: echomode.o $(EXOBJS)
	gcc -o echomode echomode.o $(EXOBJS) $(LIBHAL) -lm

$(GENJAVA): $(XMLFILES) tcal.xml genspl.awk
	xmln $(XMLFILES) tcal.xml | awk -f ../stf-sfe/stftotab.awk | \
		awk -f genspl.awk > $(GENJAVA)

templates.tar.gz: $(XMLTEMPLATES)
	tar cf - $(XMLTEMPLATES) | gzip -c > templates.tar.gz













