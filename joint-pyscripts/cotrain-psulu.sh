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
SPATIAL_POLICY_HOME=$COTRAIN_HOME/spatial-policy/psulu/

softlink(){
  echo -e "Preparing ${RED} Soft Link ${NC}"
  local cF=''
  local cdata=$1

  # Remove folder with softlinks
  # Does not remove the original files
  rm -rf $cdata/env/train/
  rm -rf $cdata/lpfiles/train/
  rm -rf $cdata/sol/train/
  mkdir $cdata/env/train/
  mkdir $cdata/lpfiles/train/
  mkdir $cdata/sol/train/

  # create soft link with data in labeled
  lndir -silent $cdata/env/labeled/ $cdata/env/train/
  lndir -silent $cdata/lpfiles/labeled/ $cdata/lpfiles/train/
  lndir -silent $cdata/sol/labeled/ $cdata/sol/train/

  if [ -z "$(find $cdata/sol/unlabeled/ ! -name '.*' -type f)" ]; then
    echo -e "${RED} Empty unlabeled sols folder ${NC}"
  else
    # Handle unlabeled data
    for cFile in $cdata/sol/unlabeled/*sol; do
      echo $cFile
      cF=$(replace $cFile "05632" "")
      label="${cF//[!0-9]/}"
      ln -s $cdata/env/unlabeled/envi$label.yaml $cdata/env/train/envi$label.yaml
      ln -s $cdata/lpfiles/unlabeled/input$label.lp $cdata/lpfiles/train/input$label.lp
      ln -s $cdata/sol/unlabeled/input$label.sol $cdata/sol/train/input$label.sol
    done
  fi
}

# Handles softlink for spatial policy
softlink_spatial(){
  echo -e "Preparing ${RED} Spatial Soft Link ${NC}"

  local cdata=$1
  local experiment=$2

  # Remove the spatial training folder
  local trainFolder=$cdata/train/
  `rm -rf $trainFolder/target.txt`
  `mkdir -p $trainFolder`

  # First fix the images
  local unlabeledFolder=$cdata/unlabeled/$experiment/
  local labeledFolder=$cdata/labeled/

  # Creating target.txt
  `cat $unlabeledFolder\target.txt > $trainFolder\target.txt`
  `cat $labeledFolder\target.txt >> $trainFolder\target.txt`
}

# ======================
# Training using SCIP
# ======================

train_scip(){
  echo -e "Training ${RED} SCIP ${NC}"
  local cdata=$1
  local n=$2
  local experiment=$3

  cd $SCIP_HOME
  `rm -rf $COTRAIN_SCRATCH/$cdata/$experiment/train/*.trj*`
  `rm -rf $COTRAIN_SCRATCH/$cdata/$experiment/valid/*.trj*`
  scripts/train_bb_both.sh -d $cdata -p 1 -n $n -x .lp -l 10 -u 11 -i 1 \
       -e $experiment -a pru_only_norm.dat -b sel_only_norm.dat
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

  cd $SCIP_HOME
  scripts/test_bb.sh -d $cdata -e $experiment -x .lp -o $outF \
      -s $policyDir/searchPolicy.$n.h5 -k $policyDir/killPolicy.$n.h5 \
      -a $policyDir/killPolicy.$n.norm.dat -b $policyDir/searchPolicy.$n.norm.dat
  cd -
}

# =========================
# Training Spatial policy
# =========================

prep_spatial_data(){
  echo -e "Preparing ${RED} Spatial data ${NC}"

  local cdata=$1
  local experiment=$2

  # Clean up output folder
  # Use experiment tag for unlabeled folder
  local spatDataFolder=$(replace $cdata "lpfiles/" "spatialData/")
  local outFolder=$COTRAIN_SCRATCH/$spatDataFolder/
  if [[ $outFolder = *"unlabeled"* ]]; then
    outFolder=$outFolder/$experiment/
  fi

  `rm -rf $outFolder/*`

  # Create arguments
  local solF=$(replace $cdata "lpfiles/" "sol/")
  local envF=$(replace $cdata "lpfiles/" "env/")

  # Create data for training
  cd $SPATIAL_POLICY_HOME
  python createSpatialData.py --i $cdata --s $solF --e $envF --o $outFolder
  cd -
}

# =========================
# Training Spatial policy
# =========================

train_spatial(){
  echo -e "Training ${RED} Spatial Policy ${NC}"
  local cdata=$1
  local modelF=$2
  local trainFolder=$cdata/train/
  local testFolder=$cdata/test/
  local validFolder=$cdata/valid/

  # Create moddel folder
  local modelDIR=$(dirname "${modelF}")
  `mkdir -p $modelDIR`

  cd $SPATIAL_POLICY_HOME
  python spatialModel.py --train $trainFolder  \
                         --test  $testFolder \
                         --valid $validFolder \
                         --f $modelF 
  cd -
}

# =========================
# Testing Spatial policy
# =========================

test_spatial(){
  echo -e "Running ${RED} Spatial Policy on Unlabled data${NC}"
  local cdata=$1
  local experiment=$2
  local modelF=$3

  # Create arguments
  local solF=$(replace $cdata "lpfiles/" "sol/")
  local envF=$(replace $cdata "lpfiles/" "env/")
  local solDir=$COTRAIN_SCRATCH/$cdata/$experiment/
  `mkdir -p $solDir/pred_sols/`

  # Create data for training
  cd $SPATIAL_POLICY_HOME
  python createSpatialData.py --i $cdata --s $solF --e $envF --o $solDir --f $modelF --m True
  cd -
}

# ==============================================
# Compare solutions to choose a good solution
# ==============================================

compare_solution(){
  echo -e "Comparing ${RED} solution between SCIP and Spatil policy ${NC}"
  local lambda_=$1
  local spatialSol=$2
  local scipSol=$3
  local lpFolder=$4
  local outFolder=$5

  # Create arguments
  local envF=$(replace $lpFolder "lpfiles/" "env/")

  # Remove previous iterations new sols
  `rm -rf $outFolder/*new.sol`

  # Now compare solutions in the two folders
  cd $COTRAIN_HOME/joint-pyscripts/

  python psulu_compare.py --p $spatialSol --s $scipSol --i $lpFolder --e $envF --o $outFolder --c $lambda_
  cd -
}

# ===================================================
# Fix lp solutions and complete them using policies
# ===================================================

complete_solution(){
  local inpF=$1
  local optSol=$2
  local policyDir=$3
  local outSol=$4
  local num=$5
  local logF=$(replace $outSol ".sol" ".log")
  echo $logF
  bin/scipdagger -r 1 -s scip.set --useSols -o $optSol -f $inpF --sol $outSol \
                 --nodepru dagger $policyDir\killPolicy.$num.h5 \
                 --nodesel policy $policyDir\searchPolicy.$num.h5 > $logF
}

remove_move_sol(){
  local cFolder=$1
  local outF=$2
  echo -e "${RED} Removing no solution sols and moving${NC}"

  # Remove no solutions sols
  cd $cFolder
  `grep -l "no solution" *full.sol | xargs rm -rf`
  cd -

  # Empty folder
  if [ -z "$(find $cFolder -name '*.full.sol' -type f | wc -l)" ]; then
    echo -e "${RED} Folder empty ${NC}"
  else
    # Clean destinaltion folder
    cd $cFolder
    #`find $outF/ -name '*.sol' | xargs rm -rf`
    `mv *full.sol $outF\\`
    cd -

    # now rename
    cd $COTRAIN_HOME"/joint-pyscripts/"
    python finalize_sols.py --i $outF
    cd -

  fi

}

fix_solution(){
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
  for solF in $solDir/*new.sol
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

  # Remove no solution sol files
  solF=$(replace $cdata "lpfiles/" "sol/")
  remove_move_sol $solDir $solF
  cd -
}

cost=0.25
data=$COTRAIN_DATA/$1/
iter=6
experiment="nn_psulu_"$cost"_cotrain_new"
spatF="spatial_model.h5"
train=true

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
  SCIP_ITR=1
  cdata_=$data/lpfiles/unlabeled/

  ##### To be run only on new dataset ####
  if [ -z "$(ls -A $COTRAIN_SCRATCH/$data/spatialData/labeled/)" ]; then
     prep_spatial_data $data/lpfiles/labeled/ $experiment
  fi
  if [ -z "$(ls -A $COTRAIN_SCRATCH/$data/spatialData/test/)" ]; then
     prep_spatial_data $data/lpfiles/test/ $experiment
  fi
  if [ -z "$(ls -A $COTRAIN_SCRATCH/$data/spatialData/valid/)" ]; then
     prep_spatial_data $data/lpfiles/valid/ $experiment
  fi
  ########################################

  for (( i=0; i<$iter; i=$i+1)); do
    cexp=$experiment$i
    policyDir=./policy/$data/lpfiles/train/$cexp/
    modelF=$COTRAIN_SCRATCH/$policyDir/$spatF
    solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

    # Create softlinks to the unlabeled and labeled data
    softlink $data

    # Train SCIP
    train_scip $data/lpfiles/ $SCIP_ITR $cexp

    #Generate performance on the unlabeled data
    test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1))

    # Prepare spatial data
    prep_spatial_data $data/lpfiles/unlabeled/ $cexp

    # Softlink spatial data
    softlink_spatial $COTRAIN_SCRATCH/$data/spatialData/ $cexp

    # Train Spatial policy
    train_spatial $COTRAIN_SCRATCH/$data/spatialData/ $modelF

    # Generate performance on the unlabeled data
    test_spatial $data/lpfiles/unlabeled/ $cexp $modelF

    ### Compare solutions
    compare_solution $cost $solDir/pred_sols/ $solDir/ $cdata_ $solDir/

    ## Fix solutions in lp format
    fix_solution $cdata_ $solDir/ $policyDir/ $(($SCIP_ITR-1))

  done

fi

###############################
