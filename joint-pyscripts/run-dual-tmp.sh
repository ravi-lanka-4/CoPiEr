init=0
iter=15

CFOLDER="psulu_mini_d"

for (( i=$init; i<$iter; i=$i+1)); do
  sh joint-pyscripts/cotrain-dualview-tmp.sh -s -d $CFOLDER | tee "logs/"$CFOLDER"self-imit"$i".txt"
  sh joint-pyscripts/cotrain-dualview-tmp.sh -d $CFOLDER | tee "logs/"$CFOLDER"cotrain"$i".txt"
  sh joint-pyscripts/cotrain-dualview-eval-tmp.sh $CFOLDER | tee "logs/"$CFOLDER"eval"$i".txt"
done

