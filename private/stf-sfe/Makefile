.SUFFIXES: .c .o .S .elf .srec .xml .html .tab .bin

vpath %.xml ../stf-apps

.xml.tab:
	xmlv $< | awk -f stftotab.awk > $*.tab

.tab.html:
	awk -v test=$* -f tabtoqry.awk $< > $*.html

#
# add tests to each here...
#
TABS      = ADC.tab memory.tab StfEg.tab atwd_pedestal.tab atwd_baseline.tab pmt_hv_ramp.tab pmt_hv_stability.tab pressure.tab
XMLFILES  = ADC.xml memory.xml StfEg.xml atwd_pedestal.xml atwd_baseline.xml pmt_hv_ramp.xml pmt_hv_stability.xml pressure.xml
HTMLFILES = ADC.html memory.html StfEg.html atwd_pedestal.html atwd_baseline.html pmt_hv_ramp.html pmt_hv_stability.html pressure.html
HTMLDIRS  = ADC memory StfEg atwd_pedestal atwd_baseline pmt_hv_ramp pmt_hv_stability pressure

all: html

clean:
	rm -f *.o *.i *.bin *.elf *.srec \
		run-tests.html gendir stfDirectory.[ch] \
		$(HTMLFILES) $(TABS) all.tab \
		memory.h atwd_pedestal.h adwd_baseline.h \
		StfEg.h \
		chkpt stftcp mk-name

test-cgi: test-cgi.c
	gcc -Wall -o test-cgi test-cgi.c

stf: stf.c
	gcc -o stf -Wall -I../../include stf.c

install: html-install

html: run-tests.html $(HTMLFILES) $(TABS) all.tab

html-dirs:
	(cd /var/www/stf/xml; mkdir -p $(HTMLDIRS))

html-install: run-tests.html index.html cgi-install html html-dirs
	cp *.html /var/www/stf
	cp all.tab /var/www/stf/xml
	cp servers /var/www/stf/xml


CGIS = run-test view-results view-summary validate-param pick-server
CGIHELPERS = mk-name chkpt stftcp sumtotable.awk common.sh qrytoxml.awk \
	restotab.awk tabtosum.awk tabtohtml.awk validate-qry.awk \
	servertab.awk

cgi: $(CGIS) $(CGIHELPERS)

cgi-install: $(CGIS) $(CGIHELPERS)
	cp $(CGIS) /usr/lib/cgi-bin/stf
	if [[ ! -f /var/www/stf/xml/uniq.txt ]]; then echo "0" > /var/www/stf/xml/uniq.txt; fi
	cp $(CGIHELPERS) "/usr/lib/cgi-bin/stf/xml/bin"

mk-name: mk-name.c
	gcc -o mk-name -Wall mk-name.c

stftcp: stftcp.c
	gcc -o stftcp -Wall -I../public stftcp.c

chkpt: chkpt.c
	gcc -o chkpt -Wall chkpt.c

all.tab: $(TABS)
	cat $(TABS) > all.tab

run-tests.html: run-tests.awk all.tab
	awk -f run-tests.awk all.tab > run-tests.html










