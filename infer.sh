#!/bin/bash
#----------------------------------------------------
# Example SLURM job script to run MPI applications on 
# TACC's Maverick system.
#
# $Id: job.mpi 1580 2013-01-08 04:10:50Z karl $
#----------------------------------------------------

#SBATCH -J myMPI                     # Job name
#SBATCH -o logs/D.out                # Name of stdout output file (%j expands to jobId)
#SBATCH -p gtx
#SBATCH -N 1                         # Total number of nodes requested (20 cores/node)
#SBATCH -n 1                         # Total number of mpi tasks requested
#SBATCH -t 24:00:00                  # Run time (hh:mm:ss) - 4 hours

# Launch the MPI executable named "a.out"

#sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_a mvc_exp_s_0.2_0.05 1550875855 100 4 600 2000 
#sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_a mvc_exp_s_0.2_0.05 1550871547 100 4 100 2000

#cd $COTRAIN_SCRATCH/retro/
#mkdir mvc_micro_a
#mv *.txt mvc_micro_a/
#cd -
#
#sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_b mvc_exp_u_0.2_0.05 1550732316 200 4 600 1500 &
#sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_b mvc_exp_maverick_two_next_z_0.5_0.05 1550865033 600 1500 &
#wait
#
#cd $COTRAIN_SCRATCH/retro/
#mkdir mvc_micro_b
#mv *.txt mvc_micro_b/
#cd -
#
#sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_c mvc_exp_nodecay_mav_a_0.3_0.05 1550319377 300 4 1200 2500 &
#sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_c mvc_exp_nodecay_mav_t_0.25_0 1550282645 300 6 1200 3000 &
#wait
#
#cd $COTRAIN_SCRATCH/retro/
#mkdir mvc_micro_c
#mv *.txt mvc_micro_c/
#cd -
#
sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_d mvc_exp_mav_u_0.2_0.05 1550686642 400 4 1200 3000 &
sh joint-pyscripts/cotrain-withRL-eval.sh mvc_micro_d mvc_exp_ba_mav_0.2_0.05 1550761196 400 4 1200 3000 &
#wait
#
#cd $COTRAIN_SCRATCH/retro/
#mkdir mvc_micro_d
#mv *.txt mvc_micro_d/
#cd -

#sh self-infer.sh

