-include config.local

ifneq ($(wildcard llvm/_build),)
# Use our version of clang (even without installing)
LLVM_BIN= $(CURDIR)/_install/bin

LLVM_CONFIG := $(LLVM_BIN)/llvm-config

export LLVM_CONFIG
compile_llvm_target = _install/lib/libclang.so
else
compile_llvm_target = skip_llvm
endif

all: llvm src examples

.PHONY: llvm src examples skip_llvm

llvm: $(compile_llvm_target)

_install/lib/libclang.so:
	$(MAKE) -C llvm/_build install

skip_llvm:
	@echo "LLVM compilation skipped"

src: llvm
	$(MAKE) -C src

examples: src
	$(MAKE) -C examples
