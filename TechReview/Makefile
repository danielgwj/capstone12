PATH  := /usr/local/apps/tex_live/current/bin/x86_64-linux:/bin:/sbin:/usr/local/bin:/usr/bin:/usr/local/apps/bin:/usr/bin/X11:/nfs/stak/students/g/gohd/bin:.

LATEX	= latex -shell-escape
BIBTEX	= bibtex
DVIPS	= dvips
DVIPDF  = dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

EXAMPLES = $(wildcard *.c)
SRC	:= $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG	= $(SRC:%.tex=%.dvi)
PSF	= $(SRC:%.tex=%.ps)
PDF	= $(SRC:%.tex=%.pdf)

all: pdf

pdf:
	latex techreview.tex
	biber techreview
	latex techreview.tex
	latex techreview.tex
	dvips -R -Poutline -t letter techreview.dvi -o techreview.ps
	ps2pdf techreview.ps

clean:
	rm -f *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.gz *.bib *.bcf *.xml

.PHONY: all show clean ps pdf showps

