glang++ : bp.cpp

	-clang++ -v -g -cc1 -fno-rtti `llvm-config --cxxflags --ldflags --libs` -lclangBasic -lclangLex -lclangDriver -lclangFrontend -lclangParse -lclangAST -lclangSema -lclangAnalysis -I /home/petr/llvm/tools/clang/include/ -I /home/petr/llvm/include/ bp.cpp
