-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)
LLVM_FLAGS := $(filter-out -DNDEBUG,$(LLVM_FLAGS))

CLANG_LIBS := -lclangParse -lclangFrontend -lclangSerialization	\
 -lclangDriver -lclangCodeGen -lclangSema -lclangAnalysis	\
 -lclangRewrite -lclangAST -lclangLex -lclangBasic -lclangEdit

visualizer.so: visualizer.cpp
	$(CXX) -g -fno-rtti -shared -Wall $(CLANG_LIBS) $(LLVM_FLAGS) -o $@ $<
