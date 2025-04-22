all: proj4.pdf

pack: proj4.zip

clean:
	rm -f proj4.pdf proj4.aux proj4.bbl proj4.blg proj4.log proj4.zip

proj4.pdf: proj4.tex citacie.bib czplain.bst
	pdflatex $<
	bibtex proj4
	pdflatex $<
	pdflatex $<

proj4.zip: proj4.tex Makefile citacie.bib czplain.bst
	zip $@ $^
