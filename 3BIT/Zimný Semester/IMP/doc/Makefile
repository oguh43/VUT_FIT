PDF = dokumentace.pdf
TEX = main.tex

LATEX = pdflatex
LATEXFLAGS = -interaction=nonstopmode -halt-on-error

all: $(PDF)

$(PDF): $(TEX)
	$(LATEX) $(LATEXFLAGS) $(TEX)
	$(LATEX) $(LATEXFLAGS) $(TEX)
	mv main.pdf $(PDF)

clean:
	rm -f *.aux *.log *.out *.toc *.lof *.lot *.fls *.fdb_latexmk *.synctex.gz

.PHONY: all clean
