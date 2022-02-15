BUILDDIR= build
PROFDIR=gprof-profile

PROFEXEC=lab1pg.a

all: jmain

jmain:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -O2 ../lab1.cpp -o lab1.a

jgprof:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -O2 -g -pg ../lab1.cpp -o $(PROFEXEC)

gprof: jcalls jannotate

jcalls:
	cd $(PROFDIR);			\
	gprof ../$(BUILDDIR)/$(PROFEXEC) | sudo ./gprof2dot.py -f prof -n0 -e0 | \
dot -Tpng -o callgraph.png

jannotate:
	cd $(PROFDIR);			\
	gprof -A ../$(BUILDDIR)/$(PROFEXEC) > callannotations.cpp

cleanup:
	rm build/lab1.a