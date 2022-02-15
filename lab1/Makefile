BUILDDIR= build
PROFDIR=gprof-profile
PROFDIR0=gprof-profile-o0

PROFEXEC=lab1pg.a
PROFEXEC0=lab1pg0.a

SCRIPT=../scripts/gprof2dot.py

all: jmain

jmain:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -O2 -march=native ../lab1.cpp -o lab1.a

jgprof:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -O2 -g -pg ../lab1.cpp -o $(PROFEXEC)

jgprof0:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -g -pg ../lab1.cpp -o $(PROFEXEC0)

gprof: jcalls jannotate

jcalls:
	cd $(PROFDIR);			\
	gprof ../$(BUILDDIR)/$(PROFEXEC) | sudo $(SCRIPT) -f prof -n0 -e0 | \
dot -Tpng -o callgraph.png

jcalls0:
	cd $(PROFDIR0);			\
	gprof ../$(BUILDDIR)/$(PROFEXEC0) | sudo $(SCRIPT) -f prof -n0 -e0 | \
dot -Tpng -o callgraph.png

jannotate:
	cd $(PROFDIR);			\
	gprof -A ../$(BUILDDIR)/$(PROFEXEC) > callannotations.cpp

cleanup:
	rm build/lab1.a