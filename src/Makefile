-include ../config.local

# Use our version of clang (even without installing)
LLVM_BIN= $(CURDIR)/../_install/bin

LLVM_CONFIG := $(shell $(LLVM_BIN)/llvm-config --cxxflags --ldflags --libs jit core)

visualizer: visualizer.cpp commandlineopt.h stringoper.h
	g++ $< -o $@ -g -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema -lclangChecker \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic \
	$(LLVM_CONFIG)

.PHONY: test
test: test.pdf

%.dot: %.cpp visualizer
	./visualizer $< -o $@

%.pdf: %.dot
	dot -Tps $< | epstopdf --filter > $@
