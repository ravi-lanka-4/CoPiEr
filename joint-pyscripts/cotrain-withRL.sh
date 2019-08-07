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

  # Remove folder with softlinks
  # Does not remove the original files
  #rm -rf $cdata/feats/train/
  #mkdir -p $cdata/feats/train/
  #mkdir -p $cdata/$ftype/train/
  mkdir -p $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/

  # create soft link with data in labeled
  #lndir -silent $cdata/feats/labeled/ $cdata/feats/train/
  rm -rf $cdata/$ftype/train/
  mkdir -p $cdata/$ftype/train/
  ln -s $cdata/$ftype/labeled/* $cdata/$ftype/train/
  for cFile in $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/*sol; do
    filename="${cFile##*/}"
    label="${filename//[!0-9]/}"
    
    for (( j=0; j<$iter+1; j=$j+1)); do # Sorry; inefficient for now, better than breaking things right?!
      if [ -f $cdata/$ftype/unlabeled$j/input"$label""$ext" ]; then
        echo "Copying: "input"$label""$ext"
        cp $cdata/$ftype/unlabeled$j/input"$label""$ext" $cdata/$ftype/train/input"$label""$ext"
      fi
    done
  done

  ## Check sols for unlabeled data and copy the other necessary data to train
  #if [ -z "$(find $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/ ! -name '.*' -type f)" ]; then
  #  echo -e "${RED} Empty unlabeled sols folder ${NC}"
  #else
  #  # Handle unlabeled data
  #  for cFile in $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/*sol; do
  #    filename="${cFile##*/}"
  #    label="${filename//[!0-9]/}"
  #    
  #    for (( j=0; j<$iter+1; j=$j+1)); do # Sorry; inefficient for now, better than breaking things right?!
  #      if [ -f $cdata/$ftype/unlabeled$j/input"$label""$ext" ]; then
  #        echo "Copying: "input"$label""$ext"
  #        cp $cdata/feats/unlabeled$j/input$label.csv $cdata/feats/train/input$label.csv
  #        cp $cdata/$ftype/unlabeled$j/input"$label""$ext" $cdata/$ftype/train/input"$label""$ext"
  #      fi
  #    done
  #  done
  #fi

  # Copy over the labeled data sols
  if [[ $ftype = *"lpfile"* ]]; then
    cp $cdata/sol/labeled/*.sol $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/
    ext=".lp"
  else
    cp $cdata/sol/labeled/*.sol $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/
    ext=".gpickle"
  fi

}

# ======================
# Training using SCIP
# ======================

train_scip(){
  echo -e "Training ${RED} SCIP ${NC}"
  local cdata=$1
  local n=$2
  local experiment=$3
  local suffix=$4
  local ftype=$5
  local iter=$6

  local cexp="$experiment"_"$iter"

  cd $SCIP_HOME
  `rm -rf $COTRAIN_SCRATCH/$cdata/$cexp/train/*.trj*`
  `rm -rf $COTRAIN_SCRATCH/$cdata/$cexp/valid/*.trj*`
  solDir=$(replace $cdata "$ftype" "sol")
  probfeats=$(replace $cdata "$ftype" "feats")

       #-f $probfeats # Jialin: Uncomment this and add as an argument for problem specific features

  scripts/train_bb_search.sh -d $cdata -p $n -n 1 -x $suffix -l 10 -u 11 -i 1 \
       -e $cexp -b sel_only_norm.dat -o $COTRAIN_SCRATCH/$solDir/train/$experiment/ 
  cd -
}

# ===============================
# Running SCIP on unlabeled data
# ===============================

test_scip(){
  echo -e "Running ${RED} SCIP on Unlabeled data ${NC}"
  local cdata=$1
  local policyDir=$2
  local outF=$3
  local experiment=$4
  local n=$5
  local suffix=$6
  local ncount=$7

  cd $SCIP_HOME
  probfeats=$(replace $cdata "$ftype" "feats")

      #-f $probfeats # Jialin: Uncomment this and add as an argument for problem specific features

  scripts/test_bb_search.sh -d $cdata -e $experiment -x $suffix -o $outF \
      -s $policyDir/searchPolicy.$n"_keras.h5" -b $policyDir/searchPolicy.$n".norm.dat" -c $ncount

  cd -
}

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

learn_bnb(){
  local cdata=$1
  local ftype=$2
  local suffix=$3
  local cexp_pre=$4
  local iter=$5
  local ncount=$6 # node count

  echo "current data: "$cdata
  echo "ftype: "$ftype
  echo "suffix: "$suffix
  echo "cexp_pre: "$cexp_pre
  echo "iter: "$iter

  cexp="$cexp_pre"_"$iter"
  cdata_=$cdata/$ftype/unlabeled$iter/

  csuf=$(replace $suffix "." "")
  policyDir=./policy/$data/$ftype/train/$cexp/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

  # Train SCIP
  train_scip $data/$ftype/ $SCIP_ITR $cexp_pre $suffix $ftype $iter

  # Generate performance on the unlabeled data
  echo -e "Running for current iteration unlabeled data"
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount &

  for (( j=0; j<$iter; j=$j+1)); do
    echo -e "Running for "$j" iteration unlabeled data"
    cdata_=$cdata/$ftype/unlabeled$j/
    test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount &
  done

  # Wait for the runs to finish
  wait

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
  . cotrain_nstep_dqn.sh $PREFIX $cexp_pre $iter $cmax_iter
  echo $sup_prob
  if [ "$self_imit" -eq "0" ]; then
    # If self imitation don't use any supervision
    echo -e "Supervised probability turning off"
    sup_prob=0
    echo -e "Supervised probability: "$sup_prob
  else
    for ((k=0; k<$iter+1; k++)); do cost=$( bc -l <<<"$cost/(1+($k*$decay))" ); done
    sup_prob=$( bc -l <<<"(($cost*($nlabeled+$nunlabeled))/$nlabeled)" )
    #sup_prob=$( bc -l <<<"($cost*$nlabeled)/($nlabeled+$nunlabeled)" )

    echo -e "Cost: "$cost
    echo -e "Supervised probability: "$sup_prob
  fi

  if [ "$BEHAVIOR_CLONE" = true ]; then
    BC_FLAG=1
  fi

  python main.py -n_step $n_step -dev_id $dev_id \
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
                 -eps_step $EPS_STEP -sup_prob $sup_prob -behavior_cloning $BC_FLAG

  #######################
  # Test spatial policy
  #######################

  echo -e "Running for current iteration unlabeled data"
  . cotrain_eval.sh $PREFIX $cexp_pre $i
  echo python evaluate.py -n_step $n_step -dev_id $dev_id -data_test $DATA_TEST \
                     -min_n $min_n -max_n $max_n -num_env $num_env \
                     -max_iter $cmax_iter -mem_size $mem_size -g_type $g_type \
                     -learning_rate $learning_rate \
                     -max_bp_iter $max_bp_iter -net_type $net_type \
                     -save_dir $out_test -embed_dim $embed_dim -batch_size $batch_size \
                     -reg_hidden $reg_hidden -momentum 0.9 \
                     -modelF $modelF -l2 0.00 -w_scale $w_scale

  python evaluate.py -n_step $n_step -dev_id $dev_id -data_test $DATA_TEST \
                     -min_n $min_n -max_n $max_n -num_env $num_env \
                     -max_iter $cmax_iter -mem_size $mem_size -g_type $g_type \
                     -learning_rate $learning_rate \
                     -max_bp_iter $max_bp_iter -net_type $net_type \
                     -save_dir $out_test -embed_dim $embed_dim -batch_size $batch_size \
                     -reg_hidden $reg_hidden -momentum 0.9 \
                     -modelF $modelF -l2 0.00 -w_scale $w_scale

  for (( j=0; j<$iter; j=$j+1)); do
    echo -e "Running for "$j" iteration unlabeled data"
    . cotrain_eval.sh $PREFIX $cexp_pre $j
    echo python evaluate.py -n_step $n_step -dev_id $dev_id -data_test $DATA_TEST \
                            -min_n $min_n -max_n $max_n -num_env $num_env \
                            -max_iter $cmax_iter -mem_size $mem_size -g_type $g_type \
                            -learning_rate $learning_rate \
                            -max_bp_iter $max_bp_iter -net_type $net_type \
                            -save_dir $out_test -embed_dim $embed_dim -batch_size $batch_size \
                            -reg_hidden $reg_hidden -momentum 0.9 \
                            -modelF $modelF -l2 0.00 -w_scale $w_scale

    python evaluate.py -n_step $n_step -dev_id $dev_id -data_test $DATA_TEST \
                       -min_n $min_n -max_n $max_n -num_env $num_env \
                       -max_iter $cmax_iter -mem_size $mem_size -g_type $g_type \
                       -learning_rate $learning_rate \
                       -max_bp_iter $max_bp_iter -net_type $net_type \
                       -save_dir $out_test -embed_dim $embed_dim -batch_size $batch_size \
                       -reg_hidden $reg_hidden -momentum 0.9 \
                       -modelF $modelF -l2 0.00 -w_scale $w_scale

  done
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

while getopts ":hd:bd:s:v:c:e:n:t:l:k:j:" arg; do
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

  lpexp=$experiment"_lp"
  gexp=$experiment"_gpickle"

  # Clear previous iteration sols
  `rm $COTRAIN_SCRATCH/$data/sol/train/"$lpexp"/*.sol`
  `rm $COTRAIN_SCRATCH/$data/sol/train/"$gexp"/*.sol`

  if [ $init -ne 0 ]; then
    # Current iteration data
    iterlpexp="$lpexp"_"$((init-1))"
    itergexp="$gexp"_"$((init-1))"

    # compare and destination 
    ccdata=$COTRAIN_SCRATCH/$data/"lpfiles/unlabeled"$init/
    compare_sol $ccdata/$iterlpexp/ $ccdata/$itergexp/ $COTRAIN_SCRATCH/$data/sol/train/$lpexp/
    ccdata=$COTRAIN_SCRATCH/$data/"mps/unlabeled"$init/ 
    compare_sol $ccdata/$itergexp $ccdata/$iterlpexp/ $COTRAIN_SCRATCH/$data/sol/train/$gexp/
  fi

  for (( i=$init; i<$iter; i=$i+1)); do
    # Current iteration data
    iterlpexp="$lpexp"_"$i"
    itergexp="$gexp"_"$i"

    # Create softlinks for the data products
    softlink $data "lpfiles" $lpexp $i
    softlink $data "gpickle" $gexp $i

    # ## Run branch and bound for linear programming formuulation
    time learn_bnb $data "lpfiles" ".lp" $lpexp $i $LPCOUNT

    #### Run Sparial policy 
    time learn_spatial $data "gpickle" ".gpickle" $gexp $i $cost $decay $SPATIAL_ITER
    
    for (( j=$init; j<$i+1; j=$j+1)); do
      # Current iteration data
      iterlpexp="$lpexp"_"$j"
      itergexp="$gexp"_"$j"

      ## Quadratic solution to lp solution
      gpickleFolder=$data/"gpickle/unlabeled"$j/
      lpFolder=$data/"lpfiles/unlabeled"$j/

      if [ "$self_imit" -gt "0" ]; then
        # Co-training

        # BnB to spatial
        `rm -R -f $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/`
        mkdir -p $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/
        cp -al $COTRAIN_SCRATCH/$lpFolder/$iterlpexp/ $COTRAIN_SCRATCH/$gpickleFolder/

        ## Spatial to BnB
        `rm -R -f $COTRAIN_SCRATCH/$lpFolder/$itergexp/`
        mkdir -p $COTRAIN_SCRATCH/$lpFolder/$itergexp/
        cp -al $COTRAIN_SCRATCH/$gpickleFolder/$itergexp/ $COTRAIN_SCRATCH/$lpFolder/

      elif [ "$self_imit" -lt "0" ]; then

        # BnB to spatial
        `rm -R -f $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/`
        mkdir -p $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/
        cp -al $COTRAIN_SCRATCH/$lpFolder/$iterlpexp/ $COTRAIN_SCRATCH/$gpickleFolder/
      fi

      # compare and destination 
      lpOut=$COTRAIN_SCRATCH/$data"/sol/train/"$lpexp/
      gOut=$COTRAIN_SCRATCH/$data"/sol/train/"$gexp/

      python joint-pyscripts/cmp_sols.py --s1 $COTRAIN_SCRATCH/$lpFolder/$iterlpexp/ \
                                         --s2 $COTRAIN_SCRATCH/$lpFolder/$itergexp/  \
                                         --d $lpOut --t 0
      python joint-pyscripts/cmp_sols.py --s1 $COTRAIN_SCRATCH/$gpickleFolder/$itergexp/ \
                                         --s2 $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/ \
                                         --d $gOut --t 0

    done
  done

  # Now save them for the future
  cd $COTRAIN_HOME/
  mkdir -p $COTRAIN_SCRATCH/db/
  lpPolicyDir=$SCIP_HOME/policy/$data/"lpfiles"/train/$lpexp"_*"
  spatPolicyDir=$SPATIAL_POLICY_HOME/policy/$data/"gpickle"/train/$gexp/
  time_stamp=$(date +%s)

  mkdir $COTRAIN_SCRATCH/db/$IDATA"_"$lpexp"_"$time_stamp/
  cp -R $lpPolicyDir/ $COTRAIN_SCRATCH/db/$IDATA"_"$lpexp"_"$time_stamp/
  mkdir $COTRAIN_SCRATCH/db/$IDATA"_"$gexp"_"$time_stamp
  cp -R $spatPolicyDir/ $COTRAIN_SCRATCH/db/$IDATA"_"$gexp"_"$time_stamp

  # Evaluation
  echo "For EVAL run the following"
  echo sh joint-pyscripts/cotrain-withRL-eval.sh $IDATA $experiment $time_stamp $LPCOUNT $iter

fi

###############################
