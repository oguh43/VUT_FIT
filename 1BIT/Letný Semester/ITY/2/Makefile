TARGET = projekt2.pdf

all: $(TARGET)

$(TARGET): projekt2.tex
	pdflatex $(LATEX_FLAGS) $<
	pdflatex $(LATEX_FLAGS) $<
clean:
	rm -f *.aux *.log *.out *.toc *.pdf
.PHONY: all clean
