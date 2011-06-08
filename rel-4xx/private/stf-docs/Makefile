TEXS = ADC-inputs.tex ADC-outputs.tex \
	atwd_pedestal-inputs.tex atwd_pedestal-outputs.tex \
	atwd_baseline-inputs.tex atwd_baseline-outputs.tex \
	pressure-inputs.tex pressure-outputs.tex \
	atwd_pulser-inputs.tex atwd_pulser-outputs.tex \
	memory-inputs.tex memory-outputs.tex

all: stf-tests.pdf

stf-tests.dvi: $(TEXS) stf-tests.tex stf-tests.ind 
	latex stf-tests.tex
	latex stf-tests.tex

stf-tests.ind: stf-tests.idx
	makeindex stf-tests.idx

stf-tests.idx: stf-tests.tex
	pdflatex stf-tests.tex

stf-tests.ps: stf-tests.dvi
	dvips -o stf-tests.ps stf-tests.dvi

stf-tests.pdf : $(TEXS) stf-tests.tex stf-tests.ind
	pdflatex stf-tests.tex
	pdflatex stf-tests.tex

install: stf-tests.pdf
	scp stf-tests.pdf glacier.lbl.gov:public_html/dom

clean:
	rm -f stf-tests.dvi stf-tests.ind stf-tests.log stf-tests.aux \
		stf-tests.ilg *.tab *-inputs.tex *-outputs.tex \
		stf-tests.idx stf-tests.out stf-tests.pdf stf-tests.toc \
		stf-tests.ps

ADC.tab: ../stf-sfe/stftotab.awk ../stf-apps/ADC.xml
	xmlv ../stf-apps/ADC.xml | \
		awk -v description="y" -f ../stf-sfe/stftotab.awk > ADC.tab

atwd_pedestal.tab: ../stf-sfe/stftotab.awk ../stf-apps/atwd_pedestal.xml
	xmlv ../stf-apps/atwd_pedestal.xml | \
		awk -v description="y" -f ../stf-sfe/stftotab.awk > atwd_pedestal.tab

atwd_baseline.tab: ../stf-sfe/stftotab.awk ../stf-apps/atwd_baseline.xml
	xmlv ../stf-apps/atwd_baseline.xml | \
		awk -v description="y" -f ../stf-sfe/stftotab.awk > atwd_baseline.tab

pressure.tab: ../stf-sfe/stftotab.awk ../stf-apps/pressure.xml
	xmlv ../stf-apps/pressure.xml | \
		awk -v description="y" -f ../stf-sfe/stftotab.awk > pressure.tab

atwd_pulser.tab: ../stf-sfe/stftotab.awk ../stf-apps/atwd_pulser.xml
	xmlv ../stf-apps/atwd_pulser.xml | \
		awk -v description="y" -f ../stf-sfe/stftotab.awk > atwd_pulser.tab

memory.tab: ../stf-sfe/stftotab.awk ../stf-apps/memory.xml
	xmlv ../stf-apps/memory.xml | \
		awk -v description="y" -f ../stf-sfe/stftotab.awk > memory.tab

ADC-inputs.tex: ADC.tab tabtotex.awk
	awk -v input=y -f tabtotex.awk ADC.tab | \
		 	sed 's/\_/\\\_/g' > ADC-inputs.tex

ADC-outputs.tex: ADC.tab tabtotex.awk
	awk -v output=y -f tabtotex.awk ADC.tab | \
		 	sed 's/\_/\\\_/g' > ADC-outputs.tex

atwd_pedestal-inputs.tex: atwd_pedestal.tab tabtotex.awk
	awk -v input=y -f tabtotex.awk atwd_pedestal.tab | \
		 	sed 's/\_/\\\_/g' > atwd_pedestal-inputs.tex

atwd_pedestal-outputs.tex: atwd_pedestal.tab tabtotex.awk
	awk -v output=y -f tabtotex.awk atwd_pedestal.tab | \
		 	sed 's/\_/\\\_/g' > atwd_pedestal-outputs.tex

atwd_baseline-inputs.tex: atwd_baseline.tab tabtotex.awk
	awk -v input=y -f tabtotex.awk atwd_baseline.tab | \
		 	sed 's/\_/\\\_/g' > atwd_baseline-inputs.tex

atwd_baseline-outputs.tex: atwd_baseline.tab tabtotex.awk
	awk -v output=y -f tabtotex.awk atwd_baseline.tab | \
		 	sed 's/\_/\\\_/g' > atwd_baseline-outputs.tex

pressure-inputs.tex: pressure.tab tabtotex.awk
	awk -v input=y -f tabtotex.awk pressure.tab | \
		 	sed 's/\_/\\\_/g' > pressure-inputs.tex

pressure-outputs.tex: pressure.tab tabtotex.awk
	awk -v output=y -f tabtotex.awk pressure.tab | \
		 	sed 's/\_/\\\_/g' > pressure-outputs.tex

atwd_pulser-inputs.tex: atwd_pulser.tab tabtotex.awk
	awk -v input=y -f tabtotex.awk atwd_pulser.tab | \
		 	sed 's/\_/\\\_/g' > atwd_pulser-inputs.tex

atwd_pulser-outputs.tex: atwd_pulser.tab tabtotex.awk
	awk -v output=y -f tabtotex.awk atwd_pulser.tab | \
		 	sed 's/\_/\\\_/g' > atwd_pulser-outputs.tex

memory-inputs.tex: memory.tab tabtotex.awk
	awk -v input=y -f tabtotex.awk memory.tab | \
		 	sed 's/\_/\\\_/g' > memory-inputs.tex

memory-outputs.tex: memory.tab tabtotex.awk
	awk -v output=y -f tabtotex.awk memory.tab | \
		 	sed 's/\_/\\\_/g' > memory-outputs.tex
