#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color

# ========================================
# Set up for cotraining
# ========================================

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -c <cost> -e <experiment> -n <iterations> -t <false/true>"
  echo "Example: scripts/compare.sh -d data/psulu/ts_10/train/ -c 0.1 -e c4w2 -n 3"
}

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

# ================================
# Creates softlinks for training
# ================================

SCIP_HOME=$COTRAIN_HOME/scip-dagger/
SPATIAL_POLICY_HOME=$COTRAIN_HOME/spatial-policy/graph_comb_opt/code/s2v_mvc/

softlink(){
  echo -e "Preparing ${RED} Soft Link ${NC}"
  local cF=''
  local cdata=$1
  local ftype=$2 #"lpfiles"
  local cexp_pre=$3
  local iter=$4

  if [[ $ftype = *"lpfile"* ]]; then
    ext=".lp"
  else
    ext=".gpickle"
  fi

  mkdir -p $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/
  mkdir -p $COTRAIN_SCRATCH/$cdata/gpickle/unlabeled/$cexp_pre/

  ## Quadratic solution to lp solution
  SRC=$cdata/gpickle/unlabeled$iter/
  cp $SRC/* $COTRAIN_SCRATCH/$cdata/gpickle/unlabeled/$cexp_pre/

  # Copy over the labeled data sols
  cp $cdata/sol/labeled/*.sol $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/
  ext=".gpickle"
}

# ===============================
# Running SCIP on unlabeled data
# ===============================

# ===================================================
# Fix lp solutions and complete them using policies
# ===================================================

compare_sol(){
  local src1=$1
  local src2=$2
  local dst=$3
  cd $COTRAIN_HOME/"joint-pyscripts/"
  python cmp_logs.py --s1 $src1 \
                     --s2 $src2 \
                     --d  $dst
  cd -
}

learn_spatial(){
  local cdata=$1
  local ftype=$2
  local suffix=$3
  local cexp_pre=$4
  local iter=$5
  local cost=$6
  local decay=$7
  local cmax_iter=$8
  local init_iter=$9


  cexp="$cexp_pre"_"$iter"
  cdata_=$cdata/$ftype/unlabeled$iter/

  csuf=$(replace $suffix "." "")
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

  ###############################
  # Train spatial policy
  ###############################

  cd $SPATIAL_POLICY_HOME
  PREFIX=$data/$ftype/

  # Load other settings
  . cotrain-RLonly.sh $PREFIX $cexp_pre $iter $cmax_iter $init_iter $init_samp

  if [ "$self_imit" -eq "0" ]; then
    # If self imitation don't use any supervision
    echo -e "Supervised probability turning off"
    sup_prob=0
    echo -e "Supervised probability: "$sup_prob
  else
    for ((k=0; k<$iter+1; k++)); do cost=$( bc -l <<<"$cost/(1+($k*$decay))" ); done
    sup_prob=$( bc -l <<<"(($cost*($nlabeled+$nunlabeled))/$nlabeled)" )

    echo -e "Cost: "$cost
    echo -e "Supervised probability: "$sup_prob
  fi

  if [ "$BEHAVIOR_CLONE" = true ]; then
    BC_FLAG=1
  fi

  python main_RLonly.py -n_step $n_step -dev_id $dev_id \
                        -min_n $min_n -max_n $max_n \
                        -num_env $num_env -max_iter $cmax_iter \
                        -mem_size $mem_size -g_type $g_type \
                        -learning_rate $learning_rate -max_bp_iter $max_bp_iter \
                        -net_type $net_type -save_dir $save_dir -embed_dim $embed_dim \
                        -batch_size $batch_size -reg_hidden $reg_hidden \
                        -momentum 0.9 -dataF $data_dir \
                        -validF $valid_dir -sup $sup \
                        -supDataF $sup_data_dir -supNumSeq $sup_seq_num \
                        -supMaxIter $sup_max_iter -supSolPrefix $sup_sol_prefix \
                        -modelF $modelF -l2 0.00 -w_scale $w_scale -citer $citer \
                        -eps_step $EPS_STEP -sup_prob $sup_prob -behavior_cloning \
                        $BC_FLAG -init_iter $init_iter -init_samp $init_samp

  cd $COTRAIN_HOME

}

data=$COTRAIN_DATA/$1/
iter=4
init=0 
LPCOUNT=100
SPATIAL_ITER=2000
cost=0.5
train=true
self_imit=1
decay=0
experiment="mvc_cotrain_search"
BEHAVIOR_CLONE=false
init_iter=1200
init_samp=100

while getopts ":hd:bd:s:v:c:e:n:t:l:k:j:i:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      IDATA=${OPTARG%/}
      data=$COTRAIN_DATA/$IDATA/
      ;;
    b)
      BEHAVIOR_CLONE=true
      ;;
    c)
      cost=${OPTARG}
      ;;
    s)
      self_imit=${OPTARG}
      ;;
    v)
      # Variable cost
      decay=${OPTARG}
      ;;
    e)
      experiment=${OPTARG}
      ;;
    n)
      iter=${OPTARG}
      ;;
    t)
      str1=${OPTARG}
      str2="False"
      shopt -s nocasematch
      case "$str1" in
         $str2 ) train=false;;
          *) train=true;;
      esac
      ;;
    l)
      LPCOUNT=${OPTARG}
      ;;
    j)
      SPATIAL_ITER=${OPTARG}
      ;;
    k)
      init_iter=${OPTARG}
      ;;
    i)
      init_samp=${OPTARG}
      ;;
    :)
      echo "ERROR: -${OPTARG} requires an argument"
      usage
      exit 1
      ;;
    ?)
      echo "ERROR: unknown option -${OPTARG}"
      usage
      exit 2
      ;;
  esac
done

experiment=$experiment"_"$cost"_"$decay

if [ "$self_imit" -eq "0" ]; then
  # Self-imitation
  echo "Self Imitation: "$self_imit
  experiment=$experiment"self_imit_search"

  if [ "$BEHAVIOR_CLONE" = true ]; then
    experiment=$experiment"_bc"
  fi

elif [ "$self_imit" -lt "0" ]; then
  echo "Transfer Imitation: "$self_imit
  experiment=$experiment"bnb_to_RL_imit_search"
fi

if [ "$train" = true ]; then

  # Presettings for sub modules
  SCIP_ITR=1 

  gexp=$experiment"_gpickle"

  # Clear previous iteration sols
  `rm $COTRAIN_SCRATCH/$data/gpickle/unlabeled/$gexp/*.gpickle`
  `rm $COTRAIN_SCRATCH/$data/sol/train/$gexp/*.sol`

  for (( i=$init; i<$iter; i=$i+1)); do
    # Current iteration data
    itergexp="$gexp"_"$i"

    # Create softlinks for the data products
    softlink $data "gpickle" $gexp $i
    echo $i

    ### Run Sparial policy 
    time learn_spatial $data "gpickle" ".gpickle" $gexp $i $cost $decay $SPATIAL_ITER $init_iter $init_samp

  done

  # Now save them for the future
  cd $COTRAIN_HOME/
  mkdir -p $COTRAIN_SCRATCH/db/
  spatPolicyDir=$SPATIAL_POLICY_HOME/policy/$data/"gpickle"/train/$gexp/
  time_stamp=$(date +%s)

  mkdir $COTRAIN_SCRATCH/db/$IDATA"_"$gexp"_"$time_stamp
  #cp -R $spatPolicyDir/ $COTRAIN_SCRATCH/db/$IDATA"_"$gexp"_"$time_stamp

  # Evaluation
  echo "For EVAL run the following"
  echo sh joint-pyscripts/cotrain-withRL-eval.sh $IDATA $experiment $time_stamp $LPCOUNT $iter

fi

###############################
