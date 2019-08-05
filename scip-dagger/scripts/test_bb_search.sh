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
nodecount=500
resultDir=$COTRAIN_SCRATCH

while getopts ":hd:s:k:e:x:m:r:g:f:a:b:n:o:c:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    o)
      resultDir=${OPTARG}
      echo "Results dir: $resultDir"
      ;;
    n)
      numPolicy=${OPTARG%/}
      echo "Number of policies: $data"
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
      isProbFeat=true
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
    c)
      nodecount=${OPTARG}
      echo "Node count: $node_count"
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
  read num <<< ${base//[^0-9]/ }
  if [[ $dagger -eq 0 ]]; then
    if [ $isProbFeat = true ]; then
        echo bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" \
        --snorm "$sel_norm" --probfeats "$prob_feats/feats"$num".csv" 
        bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" \
        --probfeats "$prob_feats"/feats"$num".csv  --snorm "$sel_norm"| tee $resultDir/$data/$experiment/$base.log  # 
    else
        echo bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" \
        --snorm "$sel_norm" 
        bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" \
        --snorm "$sel_norm" | tee $resultDir/$data/$experiment/$base.log 
    fi
  else
    sol=solution/$data/$base.sol
    if [ $isProbFeat = true ]; then
      echo bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file -o $sol \
        --nodesel dagger $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" --snorm "$sel_norm" --probfeats "$prob_feats"/feats"$num".csv
      bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file -o $sol \
        --nodesel dagger $searchPolicy --probfeats "$prob_feats"/feats"$num".csv \
        --sol $resultDir/$data/$experiment/$base".sol"  --snorm "$sel_norm" | tee $resultDir/$data/$experiment/$base.log # --snorm "$sel_norm" 
    else
      echo bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file -o $sol \
        --nodesel dagger $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" --snorm "$sel_norm" 
      bin/scipdagger -n $nodecount -r $freq -s scip.set -f $dir/$file -o $sol \
        --nodesel dagger $searchPolicy --sol $resultDir/$data/$experiment/$base".sol" --snorm "$sel_norm" | tee $resultDir/$data/$experiment/$base.log 
    fi
  fi
  echo $resultDir/$data/$experiment/$base.log
 done

