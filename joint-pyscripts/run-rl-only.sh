init=0
iter=10

CFOLDER=$1    # mvc_micro_a
EXP=$2        # mvc_a
LPCOUNT=$3
SPAT_ITER=$4
COST=$5
init_iter=$6
iter_samp=$7

for (( i=$init; i<$iter; i=$i+1)); do

  ## Decaying regularizer for imitation learning
  sh joint-pyscripts/cotrain-RLonly.sh -l $LPCOUNT -d $CFOLDER -e $EXP -c $COST -v 0.05 -j $SPAT_ITER -k $init_iter -i $iter_samp

done

