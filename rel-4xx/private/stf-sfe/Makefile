.SUFFIXES: .c .o .S .elf .srec .xml .html .tab .bin

XMLPATH=$(XMLDESCPATH)

vpath %.xml $(XMLPATH)

.xml.tab:
	xmln $< | awk -f stftotab.awk > $*.tab

.tab.html:
	awk -v test=$* -f tabtoqry.awk $< > $*.html

#
# add tests to each here...
#
TESTS = ADC memory StfEg atwd_pedestal atwd_baseline pmt_hv_ramp \
	pmt_hv_stability pmt_pv_ratio pressure pulser_spe_mpe \
	atwd_pedestal_sweep_forced atwd_pulser_spe \
	atwd_pedestal_spe atwd_clock1x_forced \
	fadc_fe_pulser fadc_baseline fadc_fe_forced \
	loopback_lc atwd_pedestal_noise

TABS      = $(TESTS:%=%.tab)
HTMLDIRS  = $(TESTS)

all: html cgi stftcp chkpt

clean:
	rm -f *.o *.i *.bin *.elf *.srec \
		$(TABS) tcal.tab all.tab \
		chkpt stftcp mk-name

test-cgi: test-cgi.c
	gcc -Wall -o test-cgi test-cgi.c

stf: stf.c
	gcc -o stf -Wall -I../../include stf.c

install: html-install

html: $(TABS) tcal.tab all.tab

html-dirs:
	(cd /var/www/stf/xml; mkdir -p $(HTMLDIRS))
	(cd /var/www/stf/xml; mkdir -p std-tests)
	(cd /var/www/stf/xml; mkdir -p std-results)

html-install: index.html cgi-install html html-dirs
	cp index.html /var/www/stf
	if [[ ! -d /var/www/stf ]]; then echo "no /var/www/stf"; fi
	if [[ ! -d /var/www/stf/xml ]]; then mkdir /var/www/stf/xml; fi
	cp all.tab /var/www/stf/xml
	cp servers /var/www/stf/xml

CGIS = run-test view-results view-summary validate-param pick-server \
	plot-array pick-dhserver alloff run-tests conf-test run-std-tests
CGIHELPERS = mk-name chkpt stftcp sumtotable.awk common.sh qrytoxml.awk \
	restotab.awk tabtohtml.awk validate-qry.awk \
	servertab.awk dhservertab.awk stfmode run-tests.awk \
	tabtoqry.awk dbtestfixup.awk

cgi: $(CGIS) $(CGIHELPERS)

cgi-install: $(CGIS) $(CGIHELPERS)
	cp $(CGIS) /usr/lib/cgi-bin/stf
	if [[ ! -d /var/www/stf/xml ]]; then mkdir /var/www/stf/xml; fi
	if [[ ! -f /var/www/stf/xml/uniq.txt ]]; then echo "0" > /var/www/stf/xml/uniq.txt; fi
	if [[ ! -d /usr/lib/cgi-bin/stf/xml/bin ]]; then mkdir -p /usr/lib/cgi-bin/stf/xml/bin; fi
	cp $(CGIHELPERS) "/usr/lib/cgi-bin/stf/xml/bin"

mk-name: mk-name.c
	gcc -o mk-name -Wall mk-name.c

stftcp: stftcp.c
	gcc -o stftcp -Wall -I../public stftcp.c

chkpt: chkpt.c
	gcc -o chkpt -Wall chkpt.c

all.tab: $(TABS) tcal.tab
	cat $(TABS) tcal.tab > all.tab
