init=0
iter=1

CFOLDER=$1    # mvc_micro_a
EXP=$2        # mvc_a
LPCOUNT=$3
SPAT_ITER=$4

for (( i=$init; i<$iter; i=$i+1)); do

  ## Decaying regularizer for imitation learning
  sh joint-pyscripts/cotrain-withRL-maxcut.sh -l $LPCOUNT -d $CFOLDER -e $EXP -c 0.5 -v 0.05 -j $SPAT_ITER

  ## RL only and self imitation
  sh joint-pyscripts/cotrain-withRL-maxcut.sh -l $LPCOUNT -d $CFOLDER -e $EXP -c 0.5 -s 0 -j $SPAT_ITER           

  # Behavior cloning RL only and se-maxcutlf imitation
  sh joint-pyscripts/cotrain-withRL-maxcut.sh -l $LPCOUNT -d $CFOLDER -e $EXP -c 0.5 -s 0 -b -j $SPAT_ITER        

  ## IL regularized data exchange f-maxcutrom IL to RL only
  sh joint-pyscripts/cotrain-withRL-maxcut.sh -l $LPCOUNT -d $CFOLDER -e $EXP -c 0.5 -v 0.05 -s -1 -j $SPAT_ITER  

done

