#!/bin/bash

set -e

scripts/train_prune_only.sh -d data/psulu/ts_10/ -p 2 -n 1 -x .lp -l 10 -u 10 -i 1 -f feats/psulu -e nn_prob_feat_real_norm_new_pru_only -a probfeat_real_pru_only_norm.dat > nn_prob_feat_real_norm_new_pru_only.txt

scripts/train_prune_only.sh -d data/hybrid_bids/bids_500 -p 2 -n 1 -x .lp -l 500 -u 500 -i 1 -e nn_hybrids_norm_pru_only -a hybrids_norm_pru_only.dat > hybrids_norm_pru_only.txt
scripts/train_prune_only.sh -d data/regions_bids/bids_1000 -p 2 -n 1 -x .lp -l 1000 -u 1000 -i 1 -e nn_regions_norm_pru_only -a regions_norm_pru_only.dat > regions_norm_pru_only.txt
