CXX = g++
CXXFLAGS = -std=c++17 -Wall -Werror -pedantic
QMAKE = qmake

.PHONY: all clean build run src-makefile pack docs clean-docs

all: build

DOXYFILE = Doxyfile

$(DOXYFILE):
	doxygen -g $(DOXYFILE)
	sed -i 's/PROJECT_NAME           = "My Project"/PROJECT_NAME           = "FSM Application"/' $(DOXYFILE)
	sed -i 's/PROJECT_BRIEF          =/PROJECT_BRIEF          = "A Finite State Machine Editor and Simulator"/' $(DOXYFILE)
	sed -i 's/OUTPUT_DIRECTORY       =/OUTPUT_DIRECTORY       = doc/' $(DOXYFILE)
	sed -i 's/EXTRACT_ALL            = NO/EXTRACT_ALL            = YES/' $(DOXYFILE)
	sed -i 's/EXTRACT_PRIVATE        = NO/EXTRACT_PRIVATE        = YES/' $(DOXYFILE)
	sed -i 's/EXTRACT_STATIC         = NO/EXTRACT_STATIC         = YES/' $(DOXYFILE)
	sed -i 's/RECURSIVE              = NO/RECURSIVE              = YES/' $(DOXYFILE)
	sed -i 's/GENERATE_LATEX         = NO/GENERATE_LATEX         = YES/' $(DOXYFILE)
	sed -i 's/UML_LOOK               = NO/UML_LOOK               = YES/' $(DOXYFILE)
	sed -i 's/CALL_GRAPH             = NO/CALL_GRAPH             = YES/' $(DOXYFILE)
	sed -i 's/CALLER_GRAPH           = NO/CALLER_GRAPH           = YES/' $(DOXYFILE)
	sed -i 's/DOT_IMAGE_FORMAT       = png/DOT_IMAGE_FORMAT       = svg/' $(DOXYFILE)
	sed -i 's/HAVE_DOT               = NO/HAVE_DOT               = YES/' $(DOXYFILE)
	sed -i 's/SOURCE_BROWSER         = NO/SOURCE_BROWSER         = YES/' $(DOXYFILE)
	sed -i 's/HTML_DYNAMIC_SECTIONS  = NO/HTML_DYNAMIC_SECTIONS  = YES/' $(DOXYFILE)
	sed -i 's/GENERATE_TREEVIEW      = NO/GENERATE_TREEVIEW      = YES/' $(DOXYFILE)
	sed -i 's/INLINE_SOURCES         = NO/INLINE_SOURCES         = YES/' $(DOXYFILE)
	sed -i 's/USE_PDFLATEX           = YES/USE_PDFLATEX           = YES/' $(DOXYFILE)
	sed -i 's/PDF_HYPERLINKS         = YES/PDF_HYPERLINKS         = YES/' $(DOXYFILE)
	sed -i 's/USE_HYPERREF           = YES/USE_HYPERREF           = YES/' $(DOXYFILE)
	sed -i 's/HYPERLINK_COLOR        = blue/HYPERLINK_COLOR        = blue/' $(DOXYFILE)

doxygen: $(DOXYFILE)
	doxygen $(DOXYFILE)
	cd doc/latex && make

clean-docs:
	rm -rf doc/
	rm -f $(DOXYFILE)

build: src-makefile
	cd src && $(MAKE)

src-makefile:
	cd src && $(QMAKE) FSMApp.pro

run: build
	./FSMApp

clean:
	cd src && $(MAKE) clean
	rm -f src/Makefile
	rm -rf build/

pack:
	zip -r xbohach00-xjenisf00-xkremee00.zip src/ examples/ README.txt Makefile ConceptualDesign.pdf
