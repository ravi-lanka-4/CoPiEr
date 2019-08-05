# ========================================
# get result from Gurobi solver 
# (with time or node limit)
# ========================================

#!/bin/bash

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -n <node_limit> -t <time_limit> -r <restriced_level> -x <suffix> -e <experiment> -p <num_threads>"
}

suffix=".lp.gz"
freq=1
nnode=0
time=0
threads=0

while getopts ":hd:n:t:x:r:e:p:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      echo "test data: $data"
      ;;
    e)
      experiment=${OPTARG}
      echo "experiment: $experiment"
      ;;
    x)
      suffix=${OPTARG}
      echo "data suffix: $suffix"
      ;;
    r)
      freq=${OPTARG}
      echo "restriced level: $freq"
      ;;
    n)
      nnode=${OPTARG}
      echo "node limit: $nnode"
      ;;
    t)
      time=${OPTARG}
      echo "time limit: $time"
      ;;
    p)
      threads=${OPTARG}
      echo "thread limit: $threads"
      ;;
    :)
      echo "ERROR: -${OPTARG} requires an argument"
      usage
      exit 1
      ;;
    ?)
      echo "ERROR: unknown option -${OPTARG}"
      usage
      exit 1
      ;;
  esac
done

resultDir=/tmp/results/
dir=$data
if ! [ -d $resultDir/$data/$experiment ]; then
  mkdir -p $resultDir/$data/$experiment
fi
for file in `ls $dir`; do
  base=`sed "s/$suffix//g" <<< $file`
  gurobi_cl NodeLimit=$nnode Threads=$threads $dir/$file &> $resultDir/$data/$experiment/$base.log 
  echo $resultDir/$data/$experiment/$base.log
done
