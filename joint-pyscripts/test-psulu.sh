#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color

# ========================================
# Set up for cotraining
# ========================================

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -c <cost> -e <experiment>"
  echo "Example: scripts/compare.sh -d data/psulu/ts_10/test/ -c 0.25 -e c4w2"
}

test_fix_solution(){
  echo -e "Completing ${RED} solution from SCIP and Spatil policy ${NC}"
  local cdata=$1
  local solDir=$2
  local policyDir=$3
  local num=$4
  local inpFname=''
  local inpF=''
  local outSolFname=''
  local outSolF=''

  # Check if the output folder is available and delete previous sols
  `mkdir -p $solDir`
  `find $solDir/ -name '*.full.sol' | xargs rm -rf`

  # For each file complete_sols 
  cd $SCIP_HOME
  for solF in $solDir/*new.sol;
  do
    basename=${solF##*/}
    echo -e "${RED}$basename${NC}"

    # Prepare input
    inpFname=$(replace $basename ".new.sol" ".lp")
    inpF=$cdata/$inpFname
    outSolFname=$(replace $basename ".new.sol" ".full.sol")
    outSolF=$solDir/$outSolFname

    # Now run SCIP to complete a solution
    complete_solution  $inpF $solF $policyDir $outSolF $num
  done
  cd - 

  cd $COTRAIN_HOME"/joint-pyscripts/"
  python postFix.py --i $solDir #--t 50
  cd -

}

#!/bin/bash
PWD="$(dirname "$0")"

# Get the functions defined in the training script but don't train
source $PWD/cotrain-psulu.sh -t false

data=$COTRAIN_DATA/psulu_small_d/
cost=0.25
experiment="nn_psulu_"$cost"_cotrain"
spatF="spatial_model.h5"

# Parse through arguments
while getopts ":hd::c:e:n:t:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      ;;
    c)
      cost=${OPTARG}
      ;;
    e)
      experiment=${OPTARG}
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

cdata_=$data/lpfiles/test/
iter=6

for (( i=0; i<$iter; i=$i+1)); do
  # Presettings for sub modules
  cexp=$experiment$i
  policyDir=./policy/$data/lpfiles/train/$cexp/
  modelF=$COTRAIN_SCRATCH/$policyDir/$spatF
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/
  SCIP_ITR=1

  #Generate performance on the unlabeled data
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1))

  # Generate performance on the unlabeled data
  test_spatial $data/lpfiles/test/ $cexp $modelF

  ## Compare solutions
  compare_solution $cost $solDir/pred_sols/ $solDir/ $cdata_ $solDir/

  ## Fix solutions in lp format
  test_fix_solution $cdata_ $solDir/ $policyDir/ $(($SCIP_ITR-1))

done


