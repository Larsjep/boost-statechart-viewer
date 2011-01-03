#LLVM_SRC=llvm

-include config.local

# Use our version of clang (even without installing)
LLVM_BIN = $(CURDIR)/_install/bin

LLVM_CONFIG := $(shell $(LLVM_BIN)/llvm-config --cxxflags --ldflags --libs all)

clang++ : bp.cpp
	$(LLVM_BIN)/clang++ -v -g -cc1 -fno-rtti $(LLVM_CONFIG)		\
	-lclangBasic -lclangLex -lclangDriver -lclangFrontend		\
	-lclangParse -lclangAST -lclangSema -lclangAnalysis bp.cpp
