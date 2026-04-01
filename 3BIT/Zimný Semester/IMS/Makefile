#/*******************************************************************************
#*                                                                              *
#*                        Brno University of Technology                         *
#*                      Faculty of Information Technology                       *
#*                                                                              *
#*                               Modely a Simulácie                             *
#*                                                                              *
#*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
#*            Author: Adam Sloboda [xsloboa00 AT stud.fit.vutbr.cz]             *
#*                                   Brno 2025                                  *
#*                                                                              *
#*             Makefile for compilation, testing and experimentation            *
#*                                                                              *
#*******************************************************************************/

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LIBS = -lsimlib -lm

TARGET = kampus_sim
SOURCES = kampus_sim.cpp
OBJECTS = $(SOURCES:.cpp=.o)

DOC_SOURCE = dokumentacia.tex
DOC_PDF = dokumentacia.pdf

RESULTS = vysledky_pracovny_den.csv vysledky_vikend.csv

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

run: $(TARGET)
	@echo "========================================="
	@echo " Spustenie simulacie - Pracovny den"
	@echo "========================================="
	./$(TARGET) --days 1
	@echo ""
	@echo "========================================="
	@echo " Spustenie simulacie - Vikend"
	@echo "========================================="
	./$(TARGET) --weekend --days 1

experiments: $(TARGET)
	@echo "Experiment 1: Pracovny den (1 den)"
	./$(TARGET) --days 1 > exp1.txt
	@echo "Experiment 2: Vikend (1 den)"
	./$(TARGET) --weekend --days 1 > exp2.txt
	@echo "Experiment 3: Pracovny tyzden (5 dni)"
	./$(TARGET) --days 5 > exp3.txt
	@echo "Experimenty dokoncene. Vysledky v exp*.txt"

doc: $(DOC_SOURCE)
	pdflatex $(DOC_SOURCE)
	pdflatex $(DOC_SOURCE)
	@echo "Dokumentacia vygenerovana: $(DOC_PDF)"

clean:
	rm -f $(OBJECTS) $(TARGET)
	rm -f $(RESULTS)
	rm -f exp*.txt
	rm -f *.aux *.log *.toc *.out

distclean: clean
	rm -f $(DOC_PDF)

pack: clean
	tar -czf 06_xbohach00_xsloboa00.tar.gz *.cpp Makefile README.md *.pdf *.py requirements.txt img/*

help:
	@echo "Dostupne ciele:"
	@echo "  make          - Skompiluje program"
	@echo "  make run      - Spusti simulaciu"
	@echo "  make experiments - Spusti vsetky experimenty"
	@echo "  make doc      - Vygeneruje dokumentaciu (PDF)"
	@echo "  make clean    - Vycisti objektove subory a vysledky"
	@echo "  make distclean- Vycisti vsetko vratane PDF"
	@echo "  make pack     - Zabali projekt do tar.gz"
	@echo "  make help     - Zobrazi tuto napovedu"

.PHONY: all run experiments doc clean distclean pack help
