#!/bin/bash

data=data/hybrid_bids/bids_500/train/
experiment=nn_hybrid_bids_datafix
n=2
suffix=.lp

resultDir=/tmp/subrahma/tmp/results/$data/$experiment/policy$n/
result=$resultDir/test.result

stats=$(./scripts/get_stats_gurobi.sh -d $data -e gurobi/node-limit/$experiment/policy$n -x $suffix | tail -n 1 | cut -d' ' -f3)
tail -n 1 $stats >> $result
