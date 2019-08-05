#!/bin/bash

g_type=barabasi_albert

data_test=$COTRAIN_DATA/mvc_test/gpickle/test/
result_root="$COTRAIN_SCRATCH"/$COTRAIN_DATA"/mvc_test/"

# max belief propagation iteration
max_bp_iter=5

# embedding size
embed_dim=64

# gpu card id
dev_id=3

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
n_step=5

min_n=100
max_n=100

num_env=1
mem_size=500000
max_iter=5000

# folder to save the trained model
save_dir=$result_root/
modelF=$save_dir/"spatModel.model"
echo $modelF

echo python evaluate.py \
    -n_step $n_step \
    -dev_id $dev_id \
    -data_test $data_test \
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
    -modelF $modelF\
    -l2 0.00 \
    -w_scale $w_scale

