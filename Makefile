all: src examples

.PHONY: src examples

src:
	$(MAKE) -C src

examples: src
	$(MAKE) -C examples
