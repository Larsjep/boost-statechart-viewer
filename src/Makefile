-include ../Makefile.config

LLVM_CONFIG ?= llvm-config

LLVM_FLAGS := $(shell $(LLVM_CONFIG) --cxxflags --ldflags --libs jit core)
LLVM_FLAGS := $(filter-out -DNDEBUG,$(LLVM_FLAGS))
LLVM_VERSION := $(shell $(LLVM_CONFIG) --version)

LIBDIR ?= $(CURDIR)

CLANG_LIBS := -lclangParse -lclangFrontend -lclangSerialization	\
 -lclangDriver -lclangCodeGen -lclangSema -lclangAnalysis	\
 -lclangAST -lclangLex -lclangBasic -lclangEdit

ifneq (,$(filter 3.0,$(LLVM_VERSION))$(filter 3.1,$(LLVM_VERSION)))
CLANG_LIBS += -lclangRewrite
endif

all: visualizer.so boost-statechart-viewer

visualizer.so: visualizer.cpp
	$(CXX) -g -fno-rtti -shared -Wall $(CLANG_LIBS) $(LLVM_FLAGS) -o $@ $<

boost-statechart-viewer: boost-statechart-viewer.sh
	sed -e s,@libdir@,$(LIBDIR), $< > $@

clean:
	rm -f boost-statechart-viewer visualizer.so
