python main.py -n_step 5 -dev_id 0 -min_n 100 -max_n 500 -num_env 1 -max_iter 2000 -mem_size 500000 -g_type barabasi_albert -learning_rate 0.0001 -max_bp_iter 5 -net_type QNet -max_iter 2000 -save_dir ./policy//home1/05632/subrahma/latest/co-training/data//mvc_micro_a//gpickle//train/mvc_a_0.5_0.05_gpickle/ -embed_dim 64 -batch_size 64 -reg_hidden 64 -momentum 0.9 -dataF /home1/05632/subrahma/latest/co-training/data//mvc_micro_a//gpickle//unlabeled0/ -validF /home1/05632/subrahma/latest/co-training/data//mvc_micro_a//gpickle//valid/ -sup 1 -supDataF /home1/05632/subrahma/latest/co-training/data//mvc_micro_a//gpickle//train/ -supNumSeq 10 -supMaxIter 5 -supSolPrefix /work/05632/subrahma/scratch///home1/05632/subrahma/latest/co-training/data//mvc_micro_a//sol//train//mvc_a_0.5_0.05_gpickle -modelF ./policy//home1/05632/subrahma/latest/co-training/data//mvc_micro_a//gpickle//train/mvc_a_0.5_0.05_gpickle//spatModel.model -l2 0.00 -w_scale 0.01 -citer 0 -eps_step 20000 -sup_prob .12500000000000000000 -behavior_cloning 0