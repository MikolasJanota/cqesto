### Makefile ---
PDF_VIEWER ?= SumatraPDF
TRGT=cqesto-description
.PHONY: r all echo clean allclean m b v plots
all: r

plots:
	cd plots ; make texs

gnuplot-lua-tikz.sty:
	./mklua-sty.sh

r:
	@latexmk -pdf -c-style-errors $(TRGT)

dist: all
	mkdir dist
	cp -v *.tex *.bib *.cls *.bst paper.pdf dist/
	cp reviews/response.pdf dist/cover_letter.pdf
	cd dist ; zip -r janota16.zip .

clean:
	@latexmk -c $(TRGT)
	@rm -f $(TRGT).bbl

allclean: clean
	@latexmk -C $(TRGT)

m:
	pdflatex paper
b:
	bibtex $(TRGT)

v: all
	$(PDF_VIEWER) $(TRGT).pdf 2>/dev/null &
### Makefile ends here
