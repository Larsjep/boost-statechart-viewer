-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

.PHONY: all clean

PDF = test.pdf StopWatch.pdf main.pdf Camera/Main.pdf Keyboard.pdf

all: $(PDF)

clean:
	rm -f $(PDF)

CLANG++ ?= $(shell $(LLVM_CONFIG) --bindir)/clang++
ifeq ($(wildcard $(CLANG++)),)
CLANG++ = clang++
endif
VISUALIZER = -Xclang -load -Xclang ../src/visualizer.so -Xclang -plugin -Xclang visualize-statechart

%.o %.dot: %.cpp ../src/visualizer.so
	$(CLANG++) $(VISUALIZER) $(CXXFLAGS) -c -o $(<:.cpp=.o) $<

%.eps: %.dot
	dot -Tps $< > $@

%.pdf: %.eps
	epstopdf $< > $@

../src/visualizer:
	$(MAKE) -C ../src
