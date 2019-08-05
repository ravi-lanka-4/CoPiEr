#!/bin/bash

# =================
# Helper functions
# =================
function replace()
{
  local myresult='nas' # Not a string ;)
  local str=$1
  local substr1=$2
  local substr2=$3
  myresult=${str/$substr1/$substr2}
  echo "$myresult"
}

######################################
# Supervised learning parameters
######################################

# supervised or RL
sup=1
# Number of steps on each instance
sup_seq_num=10 
# Number of iterations over labeled data
sup_max_iter=5
# Supervised learning data folder
sup_data_dir=$COTRAIN_DATA/mvc_test/gpickle/train/
sup_sol_dir=$(replace $sup_data_dir "gpickle" "sol")
sup_sol_prefix=$COTRAIN_SCRATCH/$sup_sol_dir/"mvc_0.25_cotrain_search_gpickle"

######################################
# Other RL related  params
######################################

g_type=barabasi_albert

result_root="$COTRAIN_SCRATCH"/$COTRAIN_DATA"/mvc_test/"

# max belief propagation iteration
max_bp_iter=5

# embedding size
embed_dim=64

# gpu card id
dev_id=0

# max batch size for training/testing
batch_size=64
net_type=QNet

# set reg_hidden=0 to make a linear regression
reg_hidden=64

# learning rate
learning_rate=0.0001

# init weights with rand normal(0, w_scale)
w_scale=0.01

# nstep
n_step=2

min_n=100
max_n=100

num_env=1
mem_size=500000
max_iter=1000

# folder to save the trained model
save_dir=$result_root

# Data folder
data_dir=$COTRAIN_DATA/mvc_test/gpickle/unlabeled/
valid_dir=$COTRAIN_DATA/mvc_test/gpickle/valid/

modelF=$save_dir/'spatModel.model'

if [ ! -e $save_dir ];
then
    mkdir -p $save_dir
fi

echo python main.py \
    -n_step $n_step \
    -dev_id $dev_id \
    -min_n $min_n \
    -max_n $max_n \
    -num_env $num_env \
    -max_iter $max_iter \
    -mem_size $mem_size \
    -g_type $g_type \
    -learning_rate $learning_rate \
    -max_bp_iter $max_bp_iter \
    -net_type $net_type \
    -max_iter $max_iter \
    -save_dir $save_dir \
    -embed_dim $embed_dim \
    -batch_size $batch_size \
    -reg_hidden $reg_hidden \
    -momentum 0.9 \
    -dataF $data_dir \
    -validF $valid_dir \
    -sup $sup \
    -supDataF $sup_data_dir \
    -supNumSeq $sup_seq_num \
    -supMaxIter $sup_max_iter \
    -supSolPrefix $sup_sol_prefix \
    -modelF $modelF \
    -l2 0.00 \
    -w_scale $w_scale 
