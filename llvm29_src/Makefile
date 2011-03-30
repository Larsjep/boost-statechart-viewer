LLVM_DIR = _install/bin

LLVM_FLAGS := $(shell $(LLVM_DIR)/llvm-config --cxxflags --ldflags --libs jit core)

visualizer: visualizer.cpp commandlineopt.h stringoper.h
	$(CXX) $< -o $@ -g -lclangParse -lclangFrontend -lclangSerialization \
	-lclangDriver -lclangCodeGen -lclangSema \
	-lclangAnalysis -lclangRewrite -lclangAST -lclangLex -lclangBasic \
	$(LLVM_FLAGS)
