LLVM_CONFIG := $(shell llvm-config --cxxflags --ldflags --libs jit core)

visualizer: visualizer.cpp commandlineopt.h stringoper.h
	$(CXX) $< -o $@ -g -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema -lclangChecker \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic \
	$(LLVM_CONFIG)

.PHONY: test
test: test.pdf

%.dot: %.cpp visualizer
	./visualizer $< -o $@

%.pdf: %.dot
	dot -Tps $< | epstopdf --filter > $@
