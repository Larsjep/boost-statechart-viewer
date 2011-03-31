include ../Makefile.config
 
LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)

visualizer: visualizer.cpp stringoper.h
	$(CXX) $< -o $@ -g -Wall -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic \
	$(LLVM_FLAGS)
