#!/bin/sh
set -e
# while [ $1 ]

for src in "$@"; do
    s=${src%.cpp}
    clang++ -Xclang -load -Xclang @libdir@/visualizer.so -Xclang -plugin -Xclang visualize-statechart -c $src
    dot -Tps $s.dot > $s.eps
    epstopdf $s.eps > $s.pdf
    rm $s.dot $s.eps
done
