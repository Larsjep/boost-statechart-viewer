-include ../Makefile.config

.PHONY: all
all: test.pdf StopWatch.pdf main.pdf

CLANG++ ?= $(shell $(LLVM_CONFIG) --bindir)/clang++
VISUALIZER = -Xclang -load -Xclang ../src/visualizer.so -Xclang -plugin -Xclang visualize-statechart

%.o %.dot: %.cpp ../src/visualizer.so
	$(CLANG++) $(VISUALIZER) -c -o $(<:.cpp=.o) $<

%.eps: %.dot
	dot -Tps $< > $@

%.pdf: %.eps
	epstopdf $< > $@

../src/visualizer:
	$(MAKE) -C ../src
