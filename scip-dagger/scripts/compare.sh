#!/bin/bash

# ========================================
# get timing result from scip-dagger log 
# then use the same time/node constraint 
# to test with Gurobi and Scip
# ========================================


usage() {
  echo "Usage: $0 -d <data_path_under_dat> -o <training_data_path_policy> -e <experiment> -n <policy_id> -x <suffix> -m <neural_nets_bool> -f <prob_feats_dir> -a <prune_norm_bool> -b <sel_norm_bool>"
  echo "Example: scripts/compare.sh -d data/psulu/ts_10/train/ -o data/psulu/ts_10/train/ -e c4w2 -n 0 -x .lp"
}

neural_nets=false
prob_feats=""
pru_norm=false
sel_norm=false
old_data=""

while getopts ":hd:o:e:x:c:w:n:m:f:a:b:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      ;;
    o)
      old_data=${OPTARG%/}
      ;;
    n)
      n=${OPTARG}
      ;;
    e)
      experiment=${OPTARG}
      ;;
    x)
      suffix=${OPTARG}
      ;;
    m)
      if [ ${OPTARG} > 0 ]; then
        neural_nets=true
      else
        neural_nets=false
      fi
      ;;
    f)
      prob_feats=${OPTARG}
      ;;
    a)
      if [ ${OPTARG} > 0 ]; then
        pru_norm=true
      else
        pru_norm=false
      fi
      ;;
    b)
      if [ ${OPTARG} > 0 ]; then
        sel_norm=true
      else
        sel_norm=false
      fi
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

read c w <<< ${experiment//[^0-9]/" "}

scratch=/tmp/
resultDir=/tmp/results/$data/$experiment/policy$n/
if ! [ -d $resultDir ]; then mkdir -p $resultDir; fi
result=$resultDir/test.result
# print header
printf "%-20s %-6s %-6s %-10s %-10s %-10s %-5s %-5s\n" \
"problem" "nnodes" "time" "DB" "PB" "opt" "ogap" "igap" > $result

# run policy
echo "running policy $experiment c=$c w=$w n=$n for $data"
echo "policy" >> $result
if [ -z $old_data ]; then
  policyDir=policy/${data%/*}/train/${experiment}/
else
  policyDir=policy/${old_data%/*}/train/${experiment}/
fi
pru_norm_file=""
if [ $pru_norm = true ]; then
  pru_norm_file=$policyDir/killPolicy.$n.norm.dat
fi
sel_norm_file=""
if [ $sel_norm = true ]; then
  sel_norm_file=$policyDir/searchPolicy.$n.norm.dat
fi

if ! [ $neural_nets = true ]; then
  ./scripts/test_bb.sh -d $data -e $experiment/policy$n -x $suffix -s $policyDir/searchPolicy.$n -k $policyDir/killPolicy.$n \
                       -f "$prob_feats" -a "$pru_norm_file" -b "$sel_norm_file"
else
  ./scripts/test_bb.sh -d $data -e $experiment/policy$n -x $suffix -s $policyDir/searchPolicy.$n.h5 -k $policyDir/killPolicy.$n.h5 \
                       -f "$prob_feats" -a "$pru_norm_file" -b "$sel_norm_file"
fi

# get statistics from each log file
echo "getting stats for $data with $experiment c=$c w=$w n=$n"
stats=$(./scripts/get_stats.sh -d $data -e $experiment/policy$n/ -n $n -x $suffix -p 1 | tail -n 1 | cut -d' ' -f3)
tail -n 1 $stats >> $result
nnodesF=$resultDir/nnodes.txt

# Get mean time and number of nodes
meanstats=$(tail -n 1 $stats | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
meanstats=($meanstats)
nnodes=${meanstats[0]}
time=${meanstats[1]}

# full scip
# echo "running scip with full feature"
# echo "scip full" >> $result
# ./scripts/test_scip.sh -d $data -x $suffix -e scip/full
# echo "getting stats for $data with scip/full"
# stats=$(./scripts/get_stats.sh -d $data -e scip/full -x $suffix | tail -n 1 | cut -d' ' -f3)
# tail -n 1 $scratch/$data/scip/full/stats >> $result 

# NIPS 2014 using the smallest scale
pru_norm_file="policy/./data/psulu/ts_10/train/nn_psulu//killPolicy.0.norm.dat"
sel_norm_file="policy/./data/psulu/ts_10/train/nn_psulu//searchPolicy.0.norm.dat"
policyDir="policy/data/psulu/ts_10/train/nn_psulu/"
./scripts/test_bb_nodes.sh -d $data -e nips2014/$experiment/policy$n -x $suffix -s $policyDir/searchPolicy.$n.h5 -k $policyDir/killPolicy.$n.h5 \
                       -f "$prob_feats" -a "$pru_norm_file" -b "$sel_norm_file" -n $nnodesF
# get statistics from each log file
echo "getting stats for $data with nips2014/$experiment"
echo ./scripts/get_stats.sh -d $data -e nips2014/$experiment/policy$n/ -n $n -x $suffix
stats=$(./scripts/get_stats.sh -d $data -e nips2014/$experiment/policy$n/ -n $n -x $suffix | tail -n 1 | cut -d' ' -f3)
tail -n 1 $stats >> $result

#
## test scip
echo "test scip with time limit $time"
echo "scip time limit" >> $result
./scripts/test_scip.sh -d $data -x $suffix -e scip/time-limit/$experiment/policy$n -t $time
echo "getting stats for $data with scip/time-limit"
stats=$(./scripts/get_stats.sh -d $data -e scip/time-limit/$experiment/policy$n -x $suffix | tail -n 1 | cut -d' ' -f3)
tail -n 1 $stats >> $result

## test scip node limit
echo "test scip with total node limit $nnodes"
echo "scip total node limit" >> $result
./scripts/test_scip_nodes.sh -d $data -x $suffix -e scip/total-node-limit/$experiment/policy$n -n $nnodesF
echo "getting stats for $data with scip/total-node-limit"
stats=$(./scripts/get_stats.sh -d $data -e scip/total-node-limit/$experiment/policy$n -x $suffix | tail -n 1 | cut -d' ' -f3)
tail -n 1 $stats >> $result

#
## test gurobi
echo "test gurobi with node limit $nnodes"
echo "gurobi node limit" >> $result
./scripts/test_gurobi_nnodes.sh -d $data -n $nnodesF -e gurobi/node-limit/$experiment/policy$n -x $suffix -p 1
echo "getting stats for $data with gurobi/node-limit"
stats=$(./scripts/get_stats_gurobi.sh -d $data -e gurobi/node-limit/$experiment/policy$n -x $suffix | tail -n 1 | cut -d' ' -f3)
tail -n 1 $stats >> $result

echo "Output: "$result
