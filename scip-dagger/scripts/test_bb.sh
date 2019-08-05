#!/bin/bash
set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -s <search_policy> -k <kill_policy> -e <experiment> -x <suffix> -r <restriced_level> -g <dagger> -f <prob_feats_dir> -a <pru_norm_file> -b <sel_norm_file> -o <output/scratch folder>"
}

suffix=".lp"
freq=1
dagger=0
prob_feats=""
pru_norm=""
sel_norm=""
isProbFeat=false
resultDir=$COTRAIN_SCRATCH

while getopts ":hd:s:k:e:x:m:r:g:f:a:b:o:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      echo "test data: $data"
      ;;
    s)
      searchPolicy=${OPTARG}
      echo "search policy: $searchPolicy"
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
      isProbFeat=false
      echo "prob features directory: $prob_feats"
      ;;
    a)
      pru_norm=${OPTARG}
      echo "prune norm file: $pru_norm"
      ;;
    b)
      sel_norm=${OPTARG}
      echo "sel norm file: $sel_norm"
      ;;
    o)
      resultDir=${OPTARG}
      echo "Results dir: $resultDir"
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

dir=$data
if ! [ -d $resultDir/$data/$experiment ]; then
  mkdir -p $resultDir/$data/$experiment
fi

for file in `ls $dir`; do
  base=`sed "s/$suffix//g" <<< $file`
  echo $file
  read num <<< ${base//[^0-9]/ }
  if [[ $dagger -eq 0 ]]; then
    if [ $isProbFeat = true ]; then
        bin/scipdagger -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --sol $resultDir/$data/$experiment/$base'.sol' \
        --nodepru policy $killPolicy --probfeats "$prob_feats/feats"$num".txt" --pnorm "$pru_norm" --snorm "$sel_norm" > $resultDir/$data/$experiment/$base'.log'
    else
        bin/scipdagger -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --sol $resultDir/$data/$experiment/$base'.sol' \
        --nodepru policy $killPolicy --pnorm "$pru_norm" --snorm "$sel_norm" > $resultDir/$data/$experiment/$base'.log'
    fi
  else
    sol=solution/$data/$base.sol
    if [ $isProbFeat = true ]; then
      bin/scipdagger -r $freq -s scip.set -f $dir/$file -o $sol --nodesel dagger $searchPolicy --sol $resultDir/$data/$experiment/$base'.sol' \
          --nodepru dagger $killPolicy --probfeats "$prob_feats/feats"$num".txt" --pnorm "$pru_norm" --snorm "$sel_norm" > $resultDir/$data/$experiment/$base'.log '
    else
      bin/scipdagger -r $freq -s scip.set -f $dir/$file -o $sol --nodesel dagger $searchPolicy --sol $resultDir/$data/$experiment/$base'.sol' \
          --nodepru dagger $killPolicy --pnorm "$pru_norm" --snorm "$sel_norm" > $resultDir/$data/$experiment/$base'.log'
    fi
  fi
  echo $resultDir/$data/$experiment/$base.log
done

