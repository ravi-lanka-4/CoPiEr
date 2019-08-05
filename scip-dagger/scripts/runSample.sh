max=1246
for (( i=1000; i <= $max; ++i ))
do
  echo "$i"
  bin/scipdagger -r 1 -s scip.set \
    -f /Users/subrahma/proj/pdf/co-training/data/psulu/lpfiles/test/input$i.lp \
    --sol /Users/subrahma/proj/pdf/co-training/scratch/12obst_15ts/pred/test/input$i.scip.sol \
    --nodepru policy ./policy/Users/subrahma/proj/pdf/co-training/data/psulu_mini/lpfiles/train/nn_psulu/killPolicy.0.h5 \
    --nodesel policy ./policy/Users/subrahma/proj/pdf/co-training/data/psulu_mini/lpfiles/train/nn_psulu/searchPolicy.0.h5

  cd ../joint-pyscripts/
  python psulu_compare.py --i lpfiles/test/input$i.lp \
                          --e env/test/envi$i.yaml \
                          --c -1 \
                          --s 12obst_15ts/pred/test/input$i.scip.sol \
                          --p 12obst_15ts/pred/test/input$i.part.sol \
                          --o 12obst_15ts/pred/test/input$i.new.sol 
  cd -

  bin/scipdagger -r 1 -s scip.set --useSols \
    -o /Users/subrahma/proj/pdf/co-training/scratch/12obst_15ts/pred/test/input$i.new.sol \
    -f /Users/subrahma/proj/pdf/co-training/data/psulu/lpfiles/test/input$i.lp \
    --sol /Users/subrahma/proj/pdf/co-training/scratch/12obst_15ts/pred/test/input$i.policy.sol \
    --nodepru dagger ./policy/Users/subrahma/proj/pdf/co-training/data/psulu_mini/lpfiles/train/nn_psulu/killPolicy.0.h5 \
    --nodesel policy ./policy/Users/subrahma/proj/pdf/co-training/data/psulu_mini/lpfiles/train/nn_psulu/searchPolicy.0.h5

  cd ../joint-pyscripts/
  python psulu_plot.py --e psulu/env/test/envi$i.yaml \
                       --s1 12obst_15ts/pred/test/input$i.scip.sol \
                       --s2 12obst_15ts/pred/test/input$i.policy.sol \
                       --s3 12obst_15ts/pred/test/input$i.new.sol \
                       --o 12obst_15ts/pred/plot/input$i.png

  cd -

done

#bin/scipdagger -r 1 -s scip.set --useSols \
#   -o $COTRAIN_SCRATCH/12obst_15ts/pred/test/input1001.part.sol  \
#   -f $COTRAIN_DATA/psulu_mini/lpfiles/test/input1001.lp \
#   --nodeprutrj $COTRAIN_SCRATCH/kill.trj \
#   --nodepru dagger ./policy/Users/subrahma/proj/pdf/co-training/data/psulu_mini/lpfiles/train/nn_psulu/killPolicy.0.h5 \
#   --sol $COTRAIN_SCRATCH/12obst_15ts/pred/test/input1001.new.sol

# Policy mode
#bin/scipdagger -r 1 -s scip.set \
#    -f $COTRAIN_DATA/psulu_mini/lpfiles/train/input0.lp \
#    --nodesel policy ./scip-dagger/policy/Users/subrahma/proj/pdf/co-training/data/psulu_mini/lpfiles/train/nn_psulu/killPolicy.0.h5 \
#    --nodepru policy policy/./data/psulu/ts_10/train/nn_psulu/killPolicy.0.h5 \
#    --pnorm policy/./data/psulu/ts_10/train/nn_psulu/killPolicy.0.norm.dat \
#    --snorm policy/./data/psulu/ts_10/train/nn_psulu/searchPolicy.0.norm.dat \
#    -j 0 -k 0
