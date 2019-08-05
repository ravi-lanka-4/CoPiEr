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
  local ftype=$2 #"lpfiles"
  local cexp_pre=$3

  # Remove folder with softlinks
  # Does not remove the original files
  rm -rf $cdata/env/train/
  rm -rf $cdata/$ftype/train/
  rm -rf $cdata/sol/train/
  mkdir -p $cdata/env/train/
  mkdir -p $cdata/$ftype/train/
  mkdir -p $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/

  # create soft link with data in labeled
  lndir -silent $cdata/env/labeled/ $cdata/env/train/
  lndir -silent $cdata/$ftype/labeled/ $cdata/$ftype/train/

  if [[ $ftype = *"lpfile"* ]]; then
    ext=".lp"
  else
    ext=".mps"
  fi

  if [ -z "$(find $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/ ! -name '.*' -type f)" ]; then
    echo -e "${RED} Empty unlabeled sols folder ${NC}"
  else
    # Handle unlabeled data
    for cFile in $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/*sol; do
      echo $cFile
      filename="${cFile##*/}"
      label="${filename//[!0-9]/}"
      
      ln -f $cdata/env/unlabeled/envi$label.yaml $cdata/env/train/envi$label.yaml
      ln -f $cdata/$ftype/unlabeled/input"$label""$ext" $cdata/$ftype/train/input"$label""$ext"
      #ln -s $cdata/sol/unlabeled/input$label.sol $cdata/sol/train/input$label.sol
    done
  fi

  if [[ $ftype = *"lpfile"* ]]; then
    cp $cdata/sol/labeled/*.sol $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/
    `rm $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/*mps.sol`
  else
    cp $cdata/sol/labeled/*.mps.sol $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/
    # rename
    for cFile in $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/*sol; do
      filename="${cFile##*/}"
      newfname=$(replace $filename ".mps" "")
      `mv $cFile $COTRAIN_SCRATCH/$cdata/sol/train/$cexp_pre/$newfname`
    done
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

  if [[ $ftype = *"lpfile"* ]]; then
    # Branch and bound for LP without problem features
    scripts/train_bb_search.sh -d $cdata -p $n -n 1 -x $suffix -l 10 -u 11 -i 1 \
         -e $cexp -b sel_only_norm.dat -o $COTRAIN_SCRATCH/$solDir/train/$experiment/
  else
    # Branch and bound for MPS with problem feats
    scripts/train_bb_search.sh -d $cdata -p $n -n 1 -x $suffix -l 10 -u 11 -i 1 \
         -e $cexp -b sel_only_norm.dat -o $COTRAIN_SCRATCH/$solDir/train/$experiment/ \
         -f $probfeats
  fi
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

# ===================================================
# Fix lp solutions and complete them using policies
# ===================================================

quad_to_lp_sol(){
  local cdata=$1
  local cexp=$2
  local policyDir=$3
  local num=$4
  local iter=$5
  local logF=$(replace $outSol ".sol" ".log")

  cexp="$cexp"_"$iter"

  mpsFolder=$(replace $cdata "lpfiles" "mps")
  local solDir=$COTRAIN_SCRATCH/$mpsFolder/$cexp/
  echo -e "${RED}Converting Quad sol to LP sol ${NC}"

  cd $COTRAIN_HOME/"joint-pyscripts/"
  for solF in $solDir/*sol; do
    basename=${solF##*/}
    echo -e "${RED}$basename${NC}"

    # lp argument
    inpFname=$(replace $basename ".sol" ".lp")
    clpF=$cdata/$lpF/$inpFname

    # envi argument
    envFname=$(replace $basename ".sol" ".yaml")
    envFname=$(replace $envFname "input" "envi")
    envFolder=$(replace $cdata "lpfiles" "env")
    envF=$envFolder/$envFname

    # new solution folder
    envFolder=$COTRAIN_SCRATCH/$cdata/$cexp/
    newSolF=$envFolder/$basename
    logFname=$(replace $basename ".sol" ".log")
    newlogF=$envFolder/$logFname

    # First creat input
    python quad_to_lp.py --i $clpF --e $envF --s $solF --o $newSolF

    # Now convert to lp format
    cd $SCIP_HOME
    bin/scipdagger -n 500 -r 1 -s scip.set --useSols -o $newSolF -f $clpF --sol $newSolF \
                   --nodepru oracle --nodesel policy \
                   $policyDir\searchPolicy.$num"_keras.json" > $newlogF
    cd -

  done
  cd -
}

lp_to_quad_sol(){
  local cdata=$1
  local exp=$2
  local iter=$3
  local policyDir=$4
  local num=$5

  exp="$exp"_"$iter"

  local lpFolder=$(replace $cdata "mps" "lpfiles")
  local solDir=$COTRAIN_SCRATCH/$lpFolder/$exp/
  echo -e "${RED}Converting LP sol to Quad sol ${NC}"

  local outFolder=$COTRAIN_SCRATCH/$cdata/$exp/
  `mkdir -p $outFolder`
  #`cp $solDir/*.sol $outFolder/`

  # Now convert to lp format
  cd $SCIP_HOME
  for solF in $solDir/*sol; 
  do
    basename=${solF##*/}
    echo -e "${RED}$basename${NC}"

    filename="${solF##*/}"
    logF=$(replace $filename ".sol" ".log")
    mpsF=$(replace $filename ".sol" ".mps")
    newlogF=$outFolder/$logF
    newSolF=$outFolder/$filename
    mpsF=$cdata/$mpsF

    # envi argument
    featFname=$(replace $basename ".sol" ".csv")
    featFname=$(replace $featFname "input" "feats")
    featFolder=$(replace $cdata "mps" "feats")
    featF=$featFolder/$featFname

    # Note the margin used to completing the solution
    bin/scipdagger -m 0.05 -n 500 -r 1 -s scip.set -o $solF -f $mpsF --sol $newSolF --probfeats "$featF" \
                   --nodepru oracle --nodesel policy $policyDir\searchPolicy.$num"_keras.json" > $newlogF
  done
  cd -
}

compare_sol(){
  local src1=$1
  local src2=$2
  local dst=$3
  local b=$4
  cd $COTRAIN_HOME/"joint-pyscripts/"
  python cmp_logs.py --s1 $src1 \
                     --s2 $src2 \
                     --d  $dst \
                     --b  $b
  cd -
}

learn_bnb(){
  local cdata=$1
  local ftype=$2
  local suffix=$3
  local cexp_pre=$4
  local iter=$5
  local ncount=$6 # node count

  cexp="$cexp_pre"_"$iter"

  # Create softlinks to the unlabeled and labeled data
  cdata_=$cdata/$ftype/unlabeled/

  csuf=$(replace $suffix "." "")
  policyDir=./policy/$data/$ftype/train/$cexp/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

  # Train SCIP
  train_scip $data/$ftype/ $SCIP_ITR $cexp_pre $suffix $ftype $iter

  # Generate performance on the unlabeled data
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount
}

MPSCOUNT=5000
LPCOUNT=500
cost=0.1
data=$COTRAIN_DATA/"psulu_mini_a"/
train=true
self_imit=false

while getopts ":hd::c:e:n:t:d:sd:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      IDATA=${OPTARG%/}
      data=$COTRAIN_DATA/$IDATA/
      ;;
    s)
      self_imit=true
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

if [ "$self_imit" = true ]; then
  # Self-imitation
  echo "Self Imitation: "$self_imit
  iter=2
  experiment="nn_psulu_"$cost"_self_imit_search"
else
  # Co-training
  iter=3
  experiment="nn_psulu_"$cost"_cotrain_search"
fi

init=0
if [ "$train" = true ]; then

  # Presettings for sub modules
  SCIP_ITR=1

  # Clear previous iteration sols
  `rm -Rf $COTRAIN_SCRATCH/$data/*`

  lpexp=$experiment"_lp"
  mpsexp=$experiment"_mps"

  if [ $init -ne 0 ]; then
    # Current iteration data
    iterlpexp="$lpexp"_"$((init-1))"
    itermpsexp="$mpsexp"_"$((init-1))"

    # compare and destination 
    ccdata=$COTRAIN_SCRATCH/$data/"lpfiles/unlabeled/" 
    compare_sol $ccdata/$iterlpexp/ $ccdata/$itermpsexp/ $COTRAIN_SCRATCH/$data/sol/train/$lpexp/
    ccdata=$COTRAIN_SCRATCH/$data/"mps/unlabeled/" 
    compare_sol $ccdata/$itermpsexp $ccdata/$iterlpexp/ $COTRAIN_SCRATCH/$data/sol/train/$mpsexp/
  fi

  for (( i=$init; i<$iter; i=$i+1)); do
    # Current iteration data
    iterlpexp="$lpexp"_"$i"
    itermpsexp="$mpsexp"_"$i"

    # Create softlinks for the data products
    softlink $data "lpfiles" $lpexp
    softlink $data "mps" $mpsexp

    ## Run branch and bound for linear programming formuulation
    learn_bnb $data "lpfiles" ".lp" $lpexp $i $LPCOUNT

    ## Run branch and bound for quadratic formuulation
    learn_bnb $data "mps" ".mps" $mpsexp $i $MPSCOUNT

    ## Quadratic solution to lp solution
    policyDir=./policy/$data/lpfiles/train/"$lpexp"_"$i"/
    quad_to_lp_sol $data/"lpfiles/unlabeled/" $mpsexp $policyDir $(($SCIP_ITR-1)) $i

    ## lp to Quadratic solution
    policyDir=./policy/$data/mps/train/"$mpsexp"_"$i"/
    lp_to_quad_sol $data/"mps/unlabeled/" $lpexp $i $policyDir $(($SCIP_ITR-1)) 

    # compare and destination 

    if [ "$self_imit" = false ]; then
      # Co-training
      ccdata=$COTRAIN_SCRATCH/$data/"lpfiles/unlabeled/" 
      compare_sol $ccdata/$iterlpexp/ $ccdata/$itermpsexp/ $COTRAIN_SCRATCH/$data/sol/train/$lpexp/ 1 # used all sols
      ccdata=$COTRAIN_SCRATCH/$data/"mps/unlabeled/" 
      compare_sol $ccdata/$itermpsexp $ccdata/$iterlpexp/ $COTRAIN_SCRATCH/$data/sol/train/$mpsexp/ 1 # use only solutions below a threshold
    else
      # Self-imitation
      ccdata=$COTRAIN_SCRATCH/$data/"lpfiles/unlabeled/" 
      mkdir -p $COTRAIN_SCRATCH/$data/sol/train/$lpexp/
      cp $ccdata/$iterlpexp/*.sol $COTRAIN_SCRATCH/$data/sol/train/$lpexp/
      ccdata=$COTRAIN_SCRATCH/$data/"mps/unlabeled/" 
      mkdir -p $COTRAIN_SCRATCH/$data/sol/train/$mpsexp/
      cp $ccdata/$itermpsexp/*.sol $COTRAIN_SCRATCH/$data/sol/train/$mpsexp/
    fi

  done

fi

###############################
