-include config.local

# Use our version of clang (even without installing)
LLVM_BIN= $(CURDIR)/_install/bin

LLVM_CONFIG := $(LLVM_BIN)/llvm-config

export LLVM_CONFIG

all: llvm src examples

.PHONY: llvm src examples

llvm: _install/lib/libclang.so

_install/lib/libclang.so:
	$(MAKE) -C llvm/_build install

src: llvm
	$(MAKE) -C src

examples: src
	$(MAKE) -C examples