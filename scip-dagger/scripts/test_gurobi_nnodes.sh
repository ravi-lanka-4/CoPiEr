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
nnodeF=0
time=0

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
      nnodeF=${OPTARG}
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
threads=1
if ! [ -d $resultDir/$data/$experiment ]; then
  mkdir -p $resultDir/$data/$experiment
fi
count=1
for file in `ls $dir`; do
  base=`sed "s/$suffix//g" <<< $file`
  nnode=`head -n $count $nnodeF | tail -n 1`
  echo $nnode
  if [ -z "${nnode}" ]; then
    count=$(($count+1))
    continue
  fi
  gurobi_cl ResultFile=$resultDir/$data/$experiment/$base.sol NodeLimit=$nnode Threads=$threads $dir/$file &> $resultDir/$data/$experiment/$base.log 
  echo $resultDir/$data/$experiment/$base.log
  count=$(($count+1))
done
