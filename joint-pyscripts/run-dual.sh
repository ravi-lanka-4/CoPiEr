init=0
iter=1

CFOLDER="psulu_mini_a"

for (( i=$init; i<$iter; i=$i+1)); do
  #sh joint-pyscripts/cotrain-dualview.sh -s -d $CFOLDER | tee "logs/"$CFOLDER"self-imit"$i".txt"
  #sh joint-pyscripts/cotrain-dualview.sh -d $CFOLDER | tee "logs/"$CFOLDER"cotrain"$i".txt"
  sh joint-pyscripts/cotrain-dualview-eval.sh $CFOLDER | tee "logs/"$CFOLDER"eval"$i".txt"
done

