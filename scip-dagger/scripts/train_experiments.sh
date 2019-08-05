#!/bin/bash

set -e

scripts/train_bb_both.sh -d $COTRAIN_DATA/psulu_mini/lpfiles/ -p 1 -n 1 -x .lp -l 10 -u 11 -i 1 -e nn_psulu -a pru_only_norm.dat -b sel_only_norm.dat
#scripts/train_prune_only.sh -d data/psulu/ts_10/ -p 2 -n 1 -x .lp -l 10 -u 10 -i 1 -e nn_feat_norm_new_pru_only -a pru_only_norm.dat > nn_feat_norm_new_pru_only_output.txt 
#scripts/train_prune_only.sh -d data/psulu/ts_10/ -p 2 -n 1 -x .lp -l 10 -u 10 -i 1 -e nn_prob_feat_norm_new_pru_only -a probfeat_pru_only_norm.dat > nn_prob_feat_norm_new_pru_only.txt
