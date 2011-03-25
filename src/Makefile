include ../Makefile.config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)

visualizer: visualizer.cpp commandlineopt.h stringoper.h
	$(CXX) $< -o $@ -g -O0 -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema -lclangChecker \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic \
	$(LLVM_FLAGS)
