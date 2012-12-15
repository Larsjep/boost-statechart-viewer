PREFIX=/usr

all: src examples

.PHONY: src examples clean install

src:
	$(MAKE) -C src

examples: src
	$(MAKE) -C examples

install:
	mkdir -p "$(DESTDIR)$(PREFIX)/lib/boost-statechart-viewer"
	install -m0644 src/visualizer.so "$(DESTDIR)$(PREFIX)/lib/boost-statechart-viewer"
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	install	src/boost-statechart-viewer "$(DESTDIR)$(PREFIX)/bin"

clean:
	$(MAKE) -C src $@
	$(MAKE) -C examples $@
