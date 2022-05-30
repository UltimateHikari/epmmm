BUILDDIR= build
PROFDIR=gprof-profile
PROFDIR0=gprof-profile-o0

PROFEXEC=lab4pg.a
PROFEXEC0=lab4pg0.a

SCRIPT=../scripts/gprof2dot.py

all: jmain

jmain:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++17 -O2 -march=native -fopt-info-vec -pg -g -pthread ../lab4.cpp -o lab4.a

jstatic:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++17 -static -O2 -march=native -fopt-info-vec -pthread ../lab4.cpp -o lab4s.a

jgccvect:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++17 -Ofast -ftree-vectorize -mavx2 -march=native -fopt-info-vec -fopenmp ../lab4.cpp -o lab4.a

vecttest:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++17 -march=native -fopt-info-vec ../tests.cpp -o tests.a

jgprof:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -O2 -g -pg ../lab4.cpp -o $(PROFEXEC)

jgprof0:
	mkdir -p $(BUILDDIR);		\
	cd $(BUILDDIR);				\
	g++ -std=c++14 -g -pg ../lab4.cpp -o $(PROFEXEC0)

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
	rm build/lab2.a; \
	rm timetest; \
	rm out.bin
