-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)

visualizer: visualizer.cpp stringoper.h iooper.h
	$(CXX) $< -o $@ -g -fno-rtti -Wall -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic -lclangEdit \
	$(LLVM_FLAGS)
