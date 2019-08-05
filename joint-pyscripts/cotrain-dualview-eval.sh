#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color

SCIP_HOME=$COTRAIN_HOME/scip-dagger/
SPATIAL_POLICY_HOME=$COTRAIN_HOME/spatial-policy/psulu/

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

eval_bnb(){
  local cdata=$1
  local ftype=$2
  local suffix=$3
  local cexp_pre=$4
  local iter=$5
  local ncount=$6 # node count
  local outF=$7
  local labelF=$8

  cexp="$cexp_pre"_"$iter"

  # Create softlinks to the unlabeled and labeled data
  csuf=$(replace $suffix "." "")
  policyDir=./policy/$data/$ftype/train/$cexp/

  # Generate performance on the unlabeled data
  cdata_=$cdata/$ftype/"test"/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount
  python joint-pyscripts/getStats.py --i $solDir --o $outF

  # on unlabeled data
  cdata_=$cdata/$ftype/"valid"/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount
  python joint-pyscripts/getStats.py --i $solDir --o $labelF
}

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

  if [[ $ftype = *"lpfile"* ]]; then
    scripts/test_bb_search.sh -d $cdata -e $experiment -x $suffix -o $outF \
        -s $policyDir/searchPolicy.$n"_keras.json" -b $policyDir/searchPolicy.$n".norm.dat" -c $ncount
  else
    scripts/test_bb_search.sh -d $cdata -e $experiment -x $suffix -o $outF \
        -s $policyDir/searchPolicy.$n"_keras.json" -b $policyDir/searchPolicy.$n".norm.dat" \
        -f $probfeats -c $ncount
  fi
  cd -
}

MPSCOUNT=5000
LPCOUNT=500
cost=0.1
data=$COTRAIN_DATA/$1/
time_stamp=$(date +%s)
outF="results/"$1"_test_"$time_stamp".txt"
labelF="results/"$1"_valid_"$time_stamp".txt"
train=true
iter=3
init=0

################ Self imitation ###############

SCIP_ITR=1
experiment="nn_psulu_"$cost"_self_imit_search"
lpexp=$experiment"_lp"
mpsexp=$experiment"_mps"
i=1 # First iteration has the self imitation policies

# Current iteration data
iterlpexp="$lpexp"_"$i"
itermpsexp="$mpsexp"_"$i"

## Run branch and bound for linear programming formuulation
eval_bnb $data "lpfiles" ".lp" $lpexp $i $LPCOUNT $outF $labelF

## Run branch and bound for quadratic formuulation
eval_bnb $data "mps" ".mps" $mpsexp $i $MPSCOUNT $outF $labelF

###############################################

################# Co-Training ################
experiment="nn_psulu_"$cost"_cotrain_search"
lpexp=$experiment"_lp"
mpsexp=$experiment"_mps"

for (( i=$init; i<$iter; i=$i+1)); do
  # Presettings for sub modules
  SCIP_ITR=1

  # Current iteration data
  iterlpexp="$lpexp"_"$i"
  itermpsexp="$mpsexp"_"$i"

  ## Run branch and bound for linear programming formuulation
  eval_bnb $data "lpfiles" ".lp" $lpexp $i $LPCOUNT $outF $labelF

  ## Run branch and bound for quadratic formuulation
  eval_bnb $data "mps" ".mps" $mpsexp $i $MPSCOUNT $outF $labelF
done

# Pack the policies for the future
cd $SCIP_HOME
tar -cvf "db/"$1"policy_"$time_stamp".tar" policy/
cd $COTRAIN_HOME

