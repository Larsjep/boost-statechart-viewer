-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)
LLVM_FLAGS := $(filter-out -DNDEBUG,$(LLVM_FLAGS))

visualizer.so: visualizer.cpp stringoper.h iooper.h
	$(CXX) $< -o $@ -g -fno-rtti -shared -fpic -Wall --std=c++11 -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic -lclangEdit \
	$(LLVM_FLAGS)
