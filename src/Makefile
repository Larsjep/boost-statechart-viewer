-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)

visualizer.so: visualizer.cpp stringoper.h iooper.h
	$(CXX) $< -o $@ -g -fno-rtti -shared -fpic -Wall -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic -lclangEdit \
	$(LLVM_FLAGS)
