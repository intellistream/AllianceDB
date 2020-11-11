#!/bin/bash
# prifile intel microarchitecture counters using VTune
expDir=/data1/xtra

id=300
rm $expDir/sink_threadId.txt
rm -rf r0*
while true
  do
  while [ ! -s  $expDir/sink_threadId.txt ]
    do
  #    echo "wait for sink id"
      sleep 0.001
  done
  pid=$(<$expDir/sink_threadId.txt)
  echo "start perf $pid..."
  #perf stat -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations -p $pid
  vtune -collect uarch-exploration -target-pid $pid
#  vtune -collect memory-consumption -data-limit 0 -target-pid $pid
  vtune -R summary -report-output $expDir/uarch/perf_$id.csv -format csv -csv-delimiter tab
  rm $expDir/sink_threadId.txt
  echo "end perf $id"
  let "id++"
done