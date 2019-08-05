#!/bin/bash

RED='\033[0;31m'
NC='\033[0m' # No Color

# Use a different folder for inference -- not to mess up with training
SCIP_HOME="$COTRAIN_HOME"scip-dagger/ 
SPATIAL_POLICY_HOME="$COTRAIN_HOME"spatial-policy/graph_comb_opt/code/s2v_mvc/

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

extract_policy(){
  local inp=$1
  local out=$2
  local TS=$3
  local ftype=$4

  `rm -Rf $out`
  `mkdir -p $out`

  # Create temp and unzip into it
  tempDir=`mktemp -d`
  cp -R $inp $tempDir/
  DST=$out/$data/$ftype/"train/"
  mkdir -p $DST

  cd $tempDir/$(basename $inp)
  cp -R * $DST/
  cd -
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
  local TS=$9

  cexp="$cexp_pre"_"$iter"

  # Create softlinks to the unlabeled and labeled data
  csuf=$(replace $suffix "." "")
  policyDir="./policy-infer"$TS/$data/$ftype/train/$cexp/
  echo $policyDir

  # Generate performance on the unlabeled data
  cdata_=$cdata/$ftype/"test"/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount
  python joint-pyscripts/getStats.py --i $solDir --o $outF --s $ftype

  # on unlabeled data
  cdata_=$cdata/$ftype/"valid"/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/
  test_scip $cdata_ $policyDir $COTRAIN_SCRATCH $cexp $(($SCIP_ITR-1)) $suffix $ncount
  python joint-pyscripts/getStats.py --i $solDir --o $labelF --s $ftype
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
        -s $policyDir/searchPolicy.$n"_keras.h5" -b $policyDir/searchPolicy.$n".norm.dat" -c $ncount
  else
    scripts/test_bb_search.sh -d $cdata -e $experiment -x $suffix -o $outF \
        -s $policyDir/searchPolicy.$n"_keras.h5" -b $policyDir/searchPolicy.$n".norm.dat" \
        -f $probfeats -c $ncount
  fi
  cd -
}

eval_spatial(){
  local cdata=$1
  local ftype=$2
  local suffix=$3
  local cexp_pre=$4
  local iter=$5
  local TS=$6

  cexp="$cexp_pre"_"$iter"
  cdata_=$cdata/$ftype/test/
  solDir=$COTRAIN_SCRATCH/$cdata_/$cexp/

  ###############################
  # Train spatial policy
  ###############################

  cd $SPATIAL_POLICY_HOME
  PREFIX=$data/$ftype/

  echo -e "Running for "$iter" iteration unlabeled data"
  . cotrain_eval.sh $PREFIX $cexp_pre $iter

  ##
  # Change some parameters
  ##
  DATA_TEST=$data/$ftype/"test/"
  policyFolder="./policy-infer"$TS/$data/$ftype/"train/"$cexp_pre/
  out_test=$COTRAIN_SCRATCH/"$DATA_TEST"/"$cexp_pre"_"$iter"/
  citer=$( bc -l <<<"$iter*$max_iter" )
  modelF=$policyFolder/"spatModel_best"$citer".model"

  python evaluate.py -n_step $n_step -dev_id $dev_id -data_test $DATA_TEST \
                          -min_n $min_n -max_n $max_n -num_env $num_env \
                          -max_iter $max_iter -mem_size $mem_size -g_type $g_type \
                          -learning_rate $learning_rate  \
                          -max_bp_iter $max_bp_iter -net_type $net_type \
                          -max_iter $max_iter -save_dir $out_test \
                          -embed_dim $embed_dim -batch_size $batch_size \
                          -reg_hidden $reg_hidden -momentum 0.9 \
                          -modelF $modelF -l2 0.00 -w_scale $w_scale
  
  cd $COTRAIN_HOME

  # Extract optimality gap
  python joint-pyscripts/getStats.py --i $out_test --o $outF --s $ftype --g
}

data=$COTRAIN_DATA/$1/
experiment=$2
time_stamp=$3
LPCOUNT=$4
iter=$5
outF=$COTRAIN_SCRATCH/"results/"$1"_"$2"_test_"$time_stamp".txt"
labelF=$COTRAIN_SCRATCH/"results/"$1"_"$2"_valid_"$time_stamp".txt"
init=0

lpexp=$experiment"_lp"
gexp=$experiment"_gpickle"

rm -Rf $SCIP_HOME"policy-infer"$time_stamp/
rm -Rf $SPATIAL_POLICY_HOME"policy-infer"$time_stamp/
extract_policy "$COTRAIN_SCRATCH"db/$1"_"$lpexp"_"$3 $SCIP_HOME"policy-infer"$time_stamp/ $time_stamp/ "lpfiles"
extract_policy "$COTRAIN_SCRATCH"db/$1"_"$gexp"_"$3 $SPATIAL_POLICY_HOME"policy-infer"$time_stamp/ $time_stamp/ "gpickle"

for (( i=$init; i<$iter; i=$i+1)); do
  # Presettings for sub modules
  SCIP_ITR=1

  # Current iteration data
  iterlpexp="$lpexp"_"$i"
  itergexp="$gexp"_"$i"

  ## Run branch and bound for linear programming formuulation
  eval_bnb $data "lpfiles" ".lp" $lpexp $i $LPCOUNT $outF $labelF $time_stamp

  ## Spatial policy inference
  eval_spatial $data "gpickle" ".gpickle" $gexp $i $time_stamp
done

rm -f $SCIP_HOME"policy-infer"$time_stamp/
rm -f $SPATIAL_POLICY_HOME"policy-infer"$time_stamp/

echo "Written to: "$outF
echo "Written to: "$labelF


