#!/bin/sh
set -e
# while [ $1 ]

if [ "$1" == "-link" ]
then
    s=/tmp/statechart
    echo > $s.cpp
    reldir=`dirname $2`
    cd $reldir
    directory=`pwd`

    for src in "${@:2}"; do
	echo "#include \""$src"\"" >> $s.cpp
    done
    clang++ -Xclang -load -Xclang /home/petr/projects/boost-statechart-viewer/src/visualizer.so -Xclang -plugin -Xclang visualize-statechart -c $s.cpp
    dot -Tps $s.dot > $s.eps
    epstopdf $s.eps > $s.pdf
    rm $s.dot $s.eps $s.cpp
    mv $s.pdf $directory
else

for src in "$@"; do
    s=${src%.*}
    clang++ -Xclang -load -Xclang @libdir@/visualizer.so -Xclang -plugin -Xclang visualize-statechart -c $src
    dot -Tps $s.dot > $s.eps
    epstopdf $s.eps > $s.pdf
    rm $s.dot $s.eps
done

fi
