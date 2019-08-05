# ========================================
# get result from Scip solver 
# (with time or node limit)
# ========================================

#!/bin/bash

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -n <node_limit> -t <time_limit> -r <restriced_level> -x <suffix> -e <experiment>"
}

suffix=".lp.gz"
freq=1
nnodeF=-1
time=-1

while getopts ":hd:n:t:x:r:e:" arg; do
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
count=1
for file in `ls $dir`; do
  base=`sed "s/$suffix//g" <<< $file`
  nnode=`head -n $count $nnodeF | tail -n 1`
  if [ -z "${nnode}" ]; then
    count=$(($count+1))
    continue
  fi
  bin/scipdagger -r $freq -s scip.set -f $dir/$file -n $nnode -t $time &> $resultDir/$data/$experiment/$base.log
  echo $resultDir/$data/$experiment/$base.log
  count=$(($count+1))
done
