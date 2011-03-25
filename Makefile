#LLVM_SRC=llvm

-include config.local

# Use our version of clang (even without installing)
LLVM_BIN= $(CURDIR)/_install/bin

LLVM_CONFIG := $(shell $(LLVM_BIN)/llvm-config --cxxflags --ldflags --libs jit core)
