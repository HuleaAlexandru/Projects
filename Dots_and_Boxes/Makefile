# Tudor Berariu, 2015

.phony: clean stats pdf

stats:
	python game_server.py

pdf: stats.tex
	pdflatex stats.tex
	rm -f stats.aux stats.log
	command -v gnome-open && gnome-open stats.pdf

stats.tex: game_server.py
	python game_server.py latex

clean:
	find . -name "stats.*" -print0 | xargs -0 rm -f
	find . -name "*~" -print0 | xargs -0 rm -f
	find . -name "*.pyc" -print0 | xargs -0 rm -f
