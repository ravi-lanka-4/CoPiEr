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
  rm -rf $cdata/feats/train/
  rm -rf $cdata/$ftype/train/
  mkdir -p $cdata/feats/train/
  mkdir -p $cdata/$ftype/train/
  mkdir -p $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/

  # create soft link with data in labeled
  lndir -silent $cdata/feats/labeled/ $cdata/feats/train/
  lndir -silent $cdata/$ftype/labeled/ $cdata/$ftype/train/

  # Check sols for unlabeled data and copy the other necessary data to train
  if [ -z "$(find $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/ ! -name '.*' -type f)" ]; then
    echo -e "${RED} Empty unlabeled sols folder ${NC}"
  else
    # Handle unlabeled data
    for cFile in $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/*sol; do
      filename="${cFile##*/}"
      label="${filename//[!0-9]/}"
      
      if [ -f $cdata/$ftype/unlabeled/input"$label""$ext" ]; then
        echo "Copying: "$cdata/$ftype/unlabeled/input"$label""$ext"
        cp $cdata/feats/unlabeled/input$label.csv $cdata/feats/train/input$label.csv
        cp $cdata/$ftype/unlabeled/input"$label""$ext" $cdata/$ftype/train/input"$label""$ext"
      fi
    done
  fi

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
  cdata_=$cdata/$ftype/unlabeled/

  csuf=$(replace $suffix "." "")
  policyDir=./policy/$data/$ftype/train/$cexp/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

  # Train SCIP
  train_scip $data/$ftype/ $SCIP_ITR $cexp_pre $suffix $ftype $iter

  # Generate performance on the unlabeled data
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount
}

learn_spatial(){
  local cdata=$1
  local ftype=$2
  local suffix=$3
  local cexp_pre=$4
  local iter=$5

  cexp="$cexp_pre"_"$iter"
  cdata_=$cdata/$ftype/unlabeled/

  csuf=$(replace $suffix "." "")
  policyDir=./policy/$data/$ftype/train/$cexp/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

  echo $data/$ftype/ $SCIP_ITR $cexp_pre $suffix $ftype $iter
  echo $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix

  ###############################
  # Train spatial policy
  ###############################

  cd $SPATIAL_POLICY_HOME
  PREFIX=$data/$ftype/

  # Load other settings
  . cotrain_nstep_dqn.sh $PREFIX $cexp_pre $iter
  python main.py -n_step $n_step -dev_id $dev_id \
                 -min_n $min_n -max_n $max_n \
                 -num_env $num_env -max_iter $max_iter \
                 -mem_size $mem_size -g_type $g_type \
                 -learning_rate $learning_rate -max_bp_iter $max_bp_iter \
                 -net_type $net_type -max_iter $max_iter \
                 -save_dir $save_dir -embed_dim $embed_dim \
                 -batch_size $batch_size -reg_hidden $reg_hidden \
                 -momentum 0.9 -dataF $data_dir \
                 -validF $valid_dir -sup $sup \
                 -supDataF $sup_data_dir -supNumSeq $sup_seq_num \
                 -supMaxIter $sup_max_iter -supSolPrefix $sup_sol_prefix \
                 -modelF $modelF -l2 0.00 -w_scale $w_scale 

  #######################
  # Test spatial policy
  #######################

  . cotrain_eval.sh $PREFIX $cexp_pre $iter
  cd $COTRAIN_HOME

}

cost=0.25
data=$COTRAIN_DATA/$1/
iter=3
init=0 
LPCOUNT=250
experiment="mvc_"$cost"_cotrain_search"
train=true

while getopts ":hd::c:e:n:t:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      IDATA=${OPTARG%/}
      data=$COTRAIN_DATA/$IDATA/
      ;;
    c)
      cost=${OPTARG}
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

if [ "$train" = true ]; then

  # Presettings for sub modules
  SCIP_ITR=1 # Jialin: Dagger iterations, change this if you want more iterations

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
    ccdata=$COTRAIN_SCRATCH/$data/"lpfiles/unlabeled/" 
    compare_sol $ccdata/$iterlpexp/ $ccdata/$itergexp/ $COTRAIN_SCRATCH/$data/sol/train/$lpexp/
    ccdata=$COTRAIN_SCRATCH/$data/"mps/unlabeled/" 
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
    learn_bnb $data "lpfiles" ".lp" $lpexp $i $LPCOUNT

    ## Run Sparial policy 
    learn_spatial $data "gpickle" ".gpickle" $gexp $i

    ## Quadratic solution to lp solution
    gpickleFolder=$data/gpickle/unlabeled/
    lpFolder=$data/lpfiles/unlabeled/

    # BnB to spatial
    mkdir -p $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/
    `rm -f $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/*sol`
    lndir -silent $COTRAIN_SCRATCH/$lpFolder/$iterlpexp/ $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/

    # Spatial to BnB
    mkdir -p $COTRAIN_SCRATCH/$lpFolder/$itergexp/
    `rm -f $COTRAIN_SCRATCH/$lpFolder/$itergexp/*sol`
    lndir -silent $COTRAIN_SCRATCH/$gpickleFolder/$itergexp/ $COTRAIN_SCRATCH/$lpFolder/$itergexp/

    # compare and destination 
    lpOut=$COTRAIN_SCRATCH/$data/sol/train/$lpexp/
    gOut=$COTRAIN_SCRATCH/$data/sol/train/$gexp/

    python joint-pyscripts/cmp_sols.py --s1 $COTRAIN_SCRATCH/$lpFolder/$iterlpexp/ \
                                       --s2 $COTRAIN_SCRATCH/$lpFolder/$itergexp/  \
                                       --d $lpOut --t 0
    python joint-pyscripts/cmp_sols.py --s1 $COTRAIN_SCRATCH/$gpickleFolder/$itergexp/ \
                                       --s2 $COTRAIN_SCRATCH/$gpickleFolder/$iterlpexp/ \
                                       --d $gOut --t 0
  done

fi

###############################
