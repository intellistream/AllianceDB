#!/bin/bash
a=1
b=2
c=$(expr $a \* 10 / $b)

echo $c

#G_SLICE=always-malloc G_DEBUG=gc-friendly  valgrind -v --tool=memcheck --leak-check=full --num-callers=40 --log-file=valgrind.log --verbose --show-leak-kinds=all ./benchmark.sh