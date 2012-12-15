-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)
LLVM_FLAGS := $(filter-out -DNDEBUG,$(LLVM_FLAGS))

LIBDIR ?= $(CURDIR)

CLANG_LIBS := -lclangParse -lclangFrontend -lclangSerialization	\
 -lclangDriver -lclangCodeGen -lclangSema -lclangAnalysis	\
 -lclangRewrite -lclangAST -lclangLex -lclangBasic -lclangEdit

all: visualizer.so boost-statechart-viewer

visualizer.so: visualizer.cpp
	$(CXX) -g -fno-rtti -shared -Wall $(CLANG_LIBS) $(LLVM_FLAGS) -o $@ $<

boost-statechart-viewer: boost-statechart-viewer.sh
	sed -e s,@libdir@,$(LIBDIR), $< > $@

clean:
	rm -f boost-statechart-viewer visualizer.so
