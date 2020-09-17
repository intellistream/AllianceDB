#!/bin/bash

pid_path=/data1/xtra

#while true
#  do
while [ ! -s  $pid_path/sink_threadId.txt ]
  do
#    echo "wait for sink id"
    sleep 0.001
done
pid=$(<$pid_path/sink_threadId.txt)
echo "start perf $pid..."
#perf stat -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations -p $pid
#vtune -collect uarch-exploration -target-pid $pid
vtune -collect memory-consumption -target-pid $pid
vtune -R summary -report-output test.csv -format csv -csv-delimiter tab
rm $pid_path/sink_threadId.txt
echo "end perf"
#done