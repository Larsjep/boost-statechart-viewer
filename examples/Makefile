.PHONY: all
all: test.pdf StopWatch.pdf main.pdf

%.dot: %.cpp ../src/visualizer
	../src/visualizer $< -o $@

%.pdf: %.dot
	dot -Tps $< | epstopdf --filter > $@

../src/visualizer:
	$(MAKE) -C ../src

