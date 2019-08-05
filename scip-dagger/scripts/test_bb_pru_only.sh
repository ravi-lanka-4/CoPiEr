#!/bin/bash
set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -k <kill_policy> -e <experiment> -x <suffix> -r <restriced_level> -g <dagger> -f <prob_feats_dir> -a <pru_norm_file>"
}

suffix=".lp"
freq=1
dagger=0
prob_feats=""
pru_norm=""
sel_norm=""

while getopts ":hd:k:e:x:m:r:g:f:a:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      echo "test data: $data"
      ;;
    k)
      killPolicy=${OPTARG}
      echo "kill policy: $killPolicy"
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
    g)
      dagger=${OPTARG}
      echo "run dagger: $dagger"
      ;;
    f)
      prob_feats=${OPTARG}
      echo "prob features directory: $prob_feats"
      ;;
    a)
      pru_norm=${OPTARG}
      echo "prune norm file: $pru_norm"
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

resultDir=/tmp/subrahma/tmp/results
dir=$data
if ! [ -d $resultDir/$data/$experiment ]; then
  mkdir -p $resultDir/$data/$experiment
fi
for file in `ls $dir`; do
  base=`sed "s/$suffix//g" <<< $file`
  if [[ $dagger -eq 0 ]]; then
    bin/scipdagger -r $freq -s scip.set -f $dir/$file \
        --nodepru policy $killPolicy &> $resultDir/$data/$experiment/$base.log \
        --probfeats "$prob_feats" --pnorm "$pru_norm"
  else
    sol=solution/$data/$base.sol
    bin/scipdagger -r $freq -s scip.set -f $dir/$file -o $sol \
        --nodepru dagger $killPolicy &> $resultDir/$data/$experiment/$base.log \
        --probfeats "$prob_feats" --pnorm "$pru_norm"
  fi
  echo $resultDir/$data/$experiment/$base.log
done

