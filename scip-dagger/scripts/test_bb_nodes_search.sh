#!/bin/bash
set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -s <search_policy> -k <kill_policy> -e <experiment> -x <suffix> -r <restriced_level> -g <dagger> -f <prob_feats_dir> -a <pru_norm_file> -b <sel_norm_file>"
}

suffix=".lp"
freq=1
dagger=0
prob_feats=""
pru_norm=""
sel_norm=""
isProbFeat=false
nnodeF=-1
numPolicy=1
while getopts ":hd:s:k:e:x:m:r:g:f:a:b:n:y:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    y)
      numPolicy=${OPTARG%/}
      echo "Number of policies: $data"
      ;;
    n)
      nnodeF=${OPTARG}
      echo "node limit: $nnode"
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

# =================
# Helper functions
# ================= 
function replace()
{
  local myresult='nas' # Not a string ;)
  local str=$1
  local substr1=$3
  local substr2=$3
  myresult=${str/$substr1/$substr2}
  echo "$myresult"
}
cdata=$(replace $data "test" "train")

resultDir=/tmp/results/
dir=$data
if ! [ -d $resultDir/$data/$experiment ]; then
  mkdir -p $resultDir/$data/$experiment
fi
count=1
for file in `ls $dir`; do
  base=`sed "s/$suffix//g" <<< $file`
  read num <<< ${base//[^0-9]/ }
  nnode=`head -n $count $nnodeF | tail -n 1`
  if [ -z "${nnode}" ]; then
    count=$(($count+1))
    continue
  fi
  if [[ $dagger -eq 0 ]]; then
    if [ $isProbFeat = true ]; then
        bin/scipdagger -n $nnode -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --np $numPolicy \
        --probfeats "$prob_feats/feats"$num".txt" --snorm "$sel_norm" &> $resultDir/$data/$experiment/$base.log 
    else
        bin/scipdagger -n $nnode -r $freq -s scip.set -f $dir/$file --nodesel policy $searchPolicy --np $numPolicy \
        --snorm "$sel_norm" > $resultDir/$data/$experiment/$base.log 
    fi
  else
    sol=solution/$data/$base.sol
    if [ $isProbFeat = true ]; then
      bin/scipdagger -n $nnode -r $freq -s scip.set -f $dir/$file -o $sol --nodesel dagger $searchPolicy --np $numPolicy \
          --probfeats "$prob_feats/feats"$num".txt" --snorm "$sel_norm" &> $resultDir/$data/$experiment/$base.log 
    else
      bin/scipdagger -n $nnode -r $freq -s scip.set -f $dir/$file -o $sol --nodesel dagger $searchPolicy --np $numPolicy \
          --snorm "$sel_norm" > $resultDir/$data/$experiment/$base.log 
    fi
  fi
  echo $resultDir/$data/$experiment/$base.log
  count=$(($count+1))
done

