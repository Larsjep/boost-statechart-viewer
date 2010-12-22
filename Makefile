LLVM_SRC=/home/petr/llvm

-include config.local

glang++ : bp.cpp
	-clang++ -v -g -cc1 -fno-rtti `llvm-config --cxxflags --ldflags	\
	--libs` -lclangBasic -lclangLex -lclangDriver -lclangFrontend	\
	-lclangParse -lclangAST -lclangSema -lclangAnalysis -I$(LLVM_SRC)/tools/clang/include/ -I $(LLVM_SRC)/include/	\
	bp.cpp
