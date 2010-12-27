#LLVM_SRC=llvm

-include config.local

clang++ : bp.cpp
	-clang++ -v -g -cc1 -fno-rtti `llvm-config --cxxflags --ldflags	\
	--libs all` -lclangBasic -lclangLex -lclangDriver -lclangFrontend	\
	-lclangParse -lclangAST -lclangSema -lclangAnalysis \
	bp.cpp
