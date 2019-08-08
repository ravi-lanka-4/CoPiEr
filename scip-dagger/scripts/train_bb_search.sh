#!/bin/bash

set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -x <suffix> -p <num_passes> -n <neural_nets_bool> -c <svm_c> -w <svm_w> -e <experiment> -r <restriced_level> -l <low> -u <high> -i <increment> -j <prune_noise> -k <sel_noise> -f <prob_feats_dir> -a <prune_norm_bool> -b <sel_norm_bool> -s <scale_igap_weights>"
  echo "Example: scripts/train_bb.sh -d data/psulu/ts_10/ -p 2 -n 1 -x .lp -l 10 -u 14 -i 1 -f feats/psulu -q <starting_scale>"
}

suffix=".lp"
freq=1
low=-1
high=-1
increment=1
neural_nets=false
pru_noise=0
sel_noise=0
prob_feats=""
pru_norm=false
sel_norm=false
scale=false
startdiff=false
startitr=false
optSol=""
nodecount=500

while getopts ":hd:p:n:c:e:w:tx:m:r:l:u:i:j:k:f:a:b:s:q:w:o:z:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      echo "training data: $data"
      ;;
    p)
      numPasses=${OPTARG}
      echo "number of passes: $numPasses"
      ;;
    o)
      optSol=${OPTARG}
      echo "Optimal Solution: $optSol"
      ;;
    n)
      if [ ${OPTARG} > 0 ]; then
        neural_nets=true
      else
        neural_nets=false
      fi
      echo "neural nets: $neural_nets"
      ;;
    s)
      if [ ${OPTARG} > 0 ]; then
        scale=true
      else
        scale=false
      fi
      echo "scale weights with igaps: $scale"
      ;;
    c)
      svmc=${OPTARG}
      echo "svm parameter c: $svmc"
      ;;
    w)
      itr=${OPTARG}
      startitr=true 
      echo "svm parameter c: $svmc"
      ;;
    q)
      startScale=${OPTARG}
      startdiff=true
      echo "Starting scale: $startScale"
      ;;
    w)
      svmw=${OPTARG}
      echo "svm parameter w: $svmw"
      ;;
    e)
      experiment=${OPTARG}
      echo "experiment name: $experiment"
      ;;
    x)
      suffix=${OPTARG}
      echo "data suffix: $suffix"
      ;;
    r)
      freq=${OPTARG}
      echo "restriced level: $freq"
      ;;
    l)
      low=${OPTARG}
      echo "folder index low: $low"
      ;;
    u)
      high=${OPTARG}
      echo "folder index high: $high"
      ;;
    i)
      increment=${OPTARG}
      echo "folder index increment: $increment"
      ;;
    j)
      pru_noise=${OPTARG}
      echo "prune noise: $pru_noise"
      ;;
    k)
      sel_noise=${OPTARG}
      echo "sel noise: $sel_noise"
      ;;
    f)
      prob_feats=${OPTARG}
      echo "prob feats folder: $prob_feats"
      ;;
    a)
      if [ ${OPTARG} > 0 ]; then
        pru_norm=true
      else
        pru_norm=false
      fi
      echo "prune norm bool: $pru_norm"
      ;;
    z)
      nodecount=${OPTARG}
      echo "Node count: $nodecount"
      ;;
    b)
      if [ ${OPTARG} > 0 ]; then
        sel_norm=true
      else
        sel_norm=false
      fi
      echo "sel norm bool: $sel_norm"
      ;;
    :)
      echo "ERROR: -${OPTARG} requires an argument"
      usage
      exit 1
      ;;
    ?)
      echo "ERROR: unknown option -${OPTARG}"
      usage
      exit 1
      ;;
  esac
done

if ! [ $startdiff = true ]; then
  startScale=$low
fi

if ! [ $startitr = true ]; then
  itr=0
fi

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

datDir="$data"
prevDatDir="$data"
prevSearchPolicy=""
prevKillPolicy=""
prevSelNormFile=""
prevPruNormFile=""
numfeats=0

loop=1
threshold=50
lbound=200

for (( j=$low; j<$high; j=$j+$increment )); do

  if [ "$low" \< "0" ]; then  
    datDir="$data"
  else
    prevDatDir=$(echo $datDir)
    datDir=$(echo $datDir | sed -e 's/'$((j-$increment))'/'$j'/g')
  fi
  solDir="$datDir/../sol/"
  scratch=$COTRAIN_SCRATCH
  tmpLog="$scratch/tmp.log"

  echo $datDir

  if [ -z $experiment ]; then
    if ! [ $neural_nets = true ]; then
      experiment=c${svmc}w${svmw}
    else
      experiment=nn
    fi
  fi

  trjDir=$scratch/$datDir/$experiment
  if ! [ -d $trjDir ]; then mkdir -p $trjDir; fi

  policyDir=policy/$datDir/train/$experiment
  policySol=$scratch/policysol/$datDir/train/$experiment
  if ! [ -d $policyDir ]; then mkdir -p $policyDir; fi
  if ! [ -d $policySol ]; then mkdir -p $policySol; fi

  prevSearchPolicy=$(echo $searchPolicy)
  prevKillPolicy=$(echo $killPolicy)
  searchPolicy=""
  killPolicy=""
  numPolicy=0
  prevSelNormFile=$(echo $selNormFile)
  selNormFile=""
  prevPruNormFile=$(echo $pruNormFile)
  pruNormFile=""
  sel_noise_iter=0
  pru_noise_iter=0

  #split_pre=("train" "valid" "test")

  searchTrjTrain=$trjDir/"train"/"search.trj"
  killTrjTrain=$trjDir/"train"/"kill.trj"
  searchTrjValid=$trjDir/"valid"/"search.trj"
  killTrjValid=$trjDir/"valid"/"kill.trj"
  searchTrjTest=$trjDir/"test"/"search.trj"
  killTrjTest=$trjDir/"test"/"kill.trj"

  # Remove trajectories if they already exist
  #split_pre=("train" "valid")
  split_pre=("train" )
  for item in ${split_pre[*]}; do
    searchTrj=$trjDir/$item/"search.trj"
    if [ -e $searchTrj.1 ]; then rm -f $searchTrj.1; rm -f $searchTrj.2; echo "rm $searchTrj"; fi
    searchTrjWeight=$searchTrj.weight
    if [ -e $searchTrjWeight ]; then rm -f $searchTrjWeight; echo "rm -f $searchTrjWeight"; fi
  done

  for i in `seq 1 $numPasses`; do

    # No prune noise when training on problems of initial size
    if [ $j -eq $low ]; then
      pru_noise_iter=0
      sel_noise_iter=0
    # When scaling up, for first pass, we use specified prune noise and for
    # every subsequent pass, we divide prune noise by 10
    else
      if [ $i -eq 1 ]; then
        pru_noise_iter=$pru_noise
        sel_noise_iter=$sel_noise
      else
        pru_noise_iter=$(echo $pru_noise_iter/10 | bc -l)
        sel_noise_iter=$(echo $sel_noise_iter/10 | bc -l)
      fi
    fi
    for item in ${split_pre[*]}; do
      curDatDir=$datDir/$item
      curSolDir=$solDir/$item
      curPolicySol=$policySol/$item
      curTrjDir=$trjDir/$item
      searchTrj=$curTrjDir/"search.trj"
      killTrj=$curTrjDir/"kill.trj"

      # Use the provided folder for training
      if [[ $item = *"train"* ]]; then
        if [ -z "$optSol" ]; then
          echo "OptSol not provided"
        else
          curSolDir=$optSol/
        fi
      fi

      for prob in `ls $curDatDir`; do
        echo $prob
        base=`sed "s/$suffix//g" <<< $prob`
        prob=$curDatDir/$prob
        sol=$curSolDir/$base.sol
  
        if [ -e "$sol" ]; then
          :
          #echo "Sol file exist: "$base
        else
          #echo "File $sol does not exist"
          continue
        fi

        killTrjIter=$curTrjDir/$base.kill.trj.$numPolicy
        searchTrjIter=$curTrjDir/$base.search.trj.$numPolicy
        probfeatsIter=""
        if ! [ -z $prob_feats ]; then
          probfeatsIter=$prob_feats/$item/feats${base:5}.csv
        fi
        echo "$prob_feats"

        if ! [ -d $curPolicySol ]; then mkdir -p $curPolicySol; fi
        if ! [ -d $curTrjDir ]; then mkdir -p $curTrjDir; fi

        if [ $startdiff = true ]; then
          if [ $startScale -gt $j ]; then
            break
          else
            startdiff=false
          fi
        fi

        if [ $startitr = true ]; then
          if [ $itr -gt $i ]; then
            break
          else
            startitr=false
          fi
        fi
                 
        #echo "pru_noise_iter: $pru_noise_iter"
        #echo "sel_noise_iter: $sel_noise_iter"
        if [ -z $searchPolicy ]; then
          # First round, no policy yet
          #echo "Gathering first iteration trajectory data"
          if [ "$datDir" != "$prevDatDir" ]; then

            # If leftover solutions from previous runs, remove them
            if [ -d $curPolicySol/$base/$i/ ]; then rm -rf $curPolicySol/$base/$i/; echo "rm -rf $curPolicySol/$base/$i/"; fi
            if ! [ -d $curPolicySol/$base/$i ]; then mkdir -p $curPolicySol/$base/$i; fi

            # Produce the solution file in Policy mode 
            for ((kk=1; kk<=$loop; kk++)); do
              cPruNormF=$(replace $prevPruNormFile "."$((numPasses-1))".norm.dat" "")
              cSelNormF=$(replace $prevSelNormFile "."$((numPasses-1))".norm.dat" "")
              #echo bin/scipdagger -r $freq -s scip_silent.set -f $prob --nodesel policy $prevSearchPolicy -n 10000 --np $numPasses \
              #                    --allsols_write $curPolicySol/$base/$i/ --sol $curPolicySol/$base.sol.$kk \
              #                    -k $sel_noise_iter --snorm "$cSelNormF" --probfeats "$probfeatsIter"
              echo $cSelNormF
              bin/scipdagger -r $freq -s scip.set -f $prob --nodesel policy $prevSearchPolicy -n 10000 --np $numPasses \
                                  --allsols_write $curPolicySol/$base/$i/ --sol $curPolicySol/$base.sol.$kk \
                                  -k $sel_noise_iter --snorm "$cSelNormF" --probfeats "$probfeatsIter"
            done

            # Get the best solutuon out of all the solutions collected
            python pyscripts/getBestSol.py -i $curPolicySol/$base.sol -o $curPolicySol/$base.sol
            if [ ! -f "$curPolicySol/$base.sol" ]; then
              continue
            fi 

            #echo bin/scipdagger -r $freq -s scip_silent.set -f $prob -o $curPolicySol/$base.sol --nodepru oracle --nodesel oracle --nodeseltrj $searchTrjIter
            #bin/scipdagger -n 200 -r $freq -s scip_silent.set -f $prob -o $curPolicySol/$base.sol --nodepru oracle --nodesel oracle --nodeseltrj $searchTrjIter \
            #                    --sscale 2 > $tmpLog

            # use the solution file to produce self imitation traces
            # echo bin/scipdagger -r $freq -s scip_silent.set -f $prob \
            #                    --nodesel dagger $prevSearchPolicy --nodepru dagger $prevKillPolicy \
            #                    --nodeseltrj $searchTrjIter --nodeprutrj $killTrjIter \
            #                    --allsols_prefix $curPolicySol/$base/$i/ -o $curPolicySol/$base.sol \
            #                    -j $pru_noise_iter -k $sel_noise_iter --probfeats "$probfeatsIter" --pnorm "$prevPruNormFile" --snorm "$prevSelNormFile"
            # bin/scipdagger -r $freq -s scip_silent.set -f $prob \
            #                    --nodesel dagger $prevSearchPolicy --nodepru dagger $prevKillPolicy \
            #                    --nodeseltrj $searchTrjIter --nodeprutrj $killTrjIter \
            #                    --allsols_prefix $curPolicySol/$base/$i/ -o $curPolicySol/$base.sol \
            #                    -j $pru_noise_iter -k $sel_noise_iter --probfeats "$probfeatsIter" --pnorm "$prevPruNormFile" --snorm "$prevSelNormFile"

            # Get Integrality gap 
            igap=$(grep "Gap" $tmpLog | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
            igap=$(echo $igap | awk '{sub(/ .*/,x)}1')    
            igap=$(echo $igap | sed 's/[eE]+*/*10^/g')  
            igap=$(echo $igap | sed 's/+//g' | bc -l)  

            re='^[0-9]*([.][0-9]+)?$'
            if ! [[ $igap =~ $re ]] ; then
               igap=1000
            fi

            # ignore data point if the gap is too large
            intgap=$( printf "%.0f" $igap )
            if [[ "$intgap" -gt "$threshold" ]] ; then
               echo "Data point ignored"
               `rm -f $curPolicySol/$base.sol`
               continue
            else
               echo "Data Point not ignored"
               echo $intgap
               echo $threshold
            fi

            # Scale the weights
            if [ $scale = true ]; then 
              igap=$(echo "$igap/100" | bc -l | awk '{ print sprintf("%.9f", $1); }')
              igap=$(echo $igap 1 | awk '{print $1 + $2}')
              echo "Scaling Factor:"$igap

              #  Multiply weights of the current output with integrality gap
              awk -v x="${igap}" -F" "  '{$1/=x;print}' $searchTrjIter.weight > $searchTrjIter.weight".tmp" && mv $searchTrjIter.weight".tmp" $searchTrjIter.weight
              awk -v x="${igap}" -F" "  '{$1/=x;print}' $killTrjIter.weight > $killTrjIter.weight".tmp" && mv $killTrjIter.weight".tmp" $killTrjIter.weight
            fi

          else
            # Oracle mode
            #echo bin/scipdagger -r $freq -s scip_silent.set -f $prob -o $sol --nodepru oracle --nodesel oracle --nodeseltrj $searchTrjIter
            echo bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodepru oracle --nodesel oracle --nodeseltrj $searchTrjIter \
                                 --pscale 2 --sscale 2 --probfeats "$probfeatsIter"| tee $tmpLog
            bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodesel oracle --nodeseltrj $searchTrjIter --nodepru oracle \
                                 --pscale 2 --sscale 2 --probfeats "$probfeatsIter"| tee $tmpLog
          fi

          if ! [ $neural_nets = true ]; then 
            cat $searchTrjIter >> $searchTrj
          else
            cat $searchTrjIter.1 >> $searchTrj.1
            cat $searchTrjIter.2 >> $searchTrj.2
          fi

          cat $searchTrjIter.weight >> $searchTrj.weight
        else
          ## Search with policy 
          #echo "Gathering trajectory data with $policy"
          ## If leftover solutions from previous runs, remove them
          #if [ -d $curPolicySol/$base/$i/ ]; then rm -r $curPolicySol/$base/$i/; echo "rm -r $curPolicySol/$base/$i/"; fi
          #if ! [ -d $curPolicySol/$base/$i ]; then mkdir -p $curPolicySol/$base/$i; fi

          ## Produce the solution file in Policy mode 
          #echo bin/scipdagger -r $freq -s scip_silent.set -f $prob --nodesel policy $searchPolicy \
          #                    --nodepru policy $killPolicy --allsols_write $curPolicySol/$base/$i/ \
          #                    -j $pru_noise_iter -k $sel_noise_iter --probfeats "$probfeatsIter" --pnorm "$pruNormFile" --snorm "$selNormFile"
          #bin/scipdagger -r $freq -s scip_silent.set -f $prob --nodesel policy $searchPolicy \
          #                    --nodepru policy $killPolicy --allsols_write $curPolicySol/$base/$i/ \
          #                    --probfeats "$probfeatsIter" --pnorm "$pruNormFile" --snorm "$selNormFile" | tee $tmpLog

          opt_sol=""
          if [ $j -eq $low ]; then
            opt_sol=$sol
          else
            opt_sol=$curPolicySol/$base.sol
          fi

          # use the solution file to produce self imitation traces          
          if [ ! -f "$opt_sol" ]; then
            continue
          fi 

       	  #pb=$(grep "Primal Bound" $tmpLog | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
       	  #pb=$(echo $pb | awk '{sub(/ .*/,x)}1')    
       	  #pb=$(echo $pb | sed 's/[eE]+*/*10^/g')
       	  #pb=$(echo $pb | sed 's/+//g' | bc -l)
       	  #echo "Primal Bound: "$pb

      	  ## Get Integrality gap 
          #opt=$(grep "bjective" $opt_sol | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
          #opt=$(echo $opt | awk '{sub(/ .*/,x)}1')
          #opt=$(echo $opt | sed 's/[eE]+*/*10^/g')
          #opt=$(echo $opt | sed 's/+//g' | bc -l)

          #ogap=$(echo "$pb $opt" | awk 'function abs(x){return ((x < 0.0) ? -x : x)} {print abs(100*($1-$2)/$2)}')
          #ogap=$(echo $ogap | awk '{sub(/ .*/,x)}1')
          #ogap=$(echo $ogap | sed 's/[eE]+*/*10^/g')
          #ogap=$(echo $ogap | sed 's/+//g' | bc -l)
          #echo "Integrality gap: "$ogap
          #
          #re='^[0-9]*([.][0-9]+)?$'
          #if ! [[ $ogap =~ $re ]] ; then
          #   ogap=100
          #fi
          #
          ## ignore data point if the gap is too large
          #ogap=$( printf "%.0f" $ogap )
          #echo "Optimality Gap: "$ogap 
          ##if [[ "$ogap" -lt "$lbound" ]] ; then
          ##   continue
          ##fi

          # --allsols_prefix $curPolicySol/$base/$i/ 
          cPruNormF=$(replace $pruNormFile "."$((numPolicy-1))".norm.dat" "")
          cSelNormF=$(replace $selNormFile "."$((numPolicy-1))".norm.dat" "")
          #echo bin/scipdagger -r $freq -s scip_silent.set -f $prob -n 1000 \
          #                  --nodesel dagger $searchPolicy --nodeseltrj $searchTrjIter \
          #                  -o $opt_sol -k $sel_noise_iter --np $((i-1)) \
          #                  --pnorm "$cPruNormF" --snorm "$cSelNormF"

          echo bin/scipdagger -r $freq -s scip.set -f $prob -b 0.3 -n $nodecount\
                            --nodesel dagger $searchPolicy --nodeseltrj $searchTrjIter \
                            -o $opt_sol --snorm "$selNormFile" --probfeats "$probfeatsIter" | tee $tmpLog

          echo $selNormFile
          bin/scipdagger -r $freq -s scip.set -f $prob -b 0.3 -n $nodecount \
                            --nodesel dagger $searchPolicy --nodeseltrj $searchTrjIter \
                            -o $opt_sol --snorm "$selNormFile" --probfeats "$probfeatsIter" | tee $tmpLog

          if [ $scale = true ]; then 
            # Get Integrality gap 
            igap=$(grep "Gap" $tmpLog | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
            igap=$(echo $igap | awk '{sub(/ .*/,x)}1')    
            igap=$(echo $igap | sed 's/[eE]+*/*10^/g')  
            igap=$(echo $igap | sed 's/+//g' | bc -l)  

            re='^[0-9]+([.][0-9]+)?$'
            if ! [[ $igap =~ $re ]] ; then
               igap=1000
            fi
            igap=$(echo "$igap/100" | bc -l | awk '{ print sprintf("%.9f", $1); }')
            igap=$(echo $igap 1 | awk '{print $1 + $2}')
            echo "Scaling Factor:"$igap

            #  Multiply weights of the current output with integrality gap
            awk -v x="${igap}" -F" "  '{$1/=x;print}' $searchTrjIter.weight \
                          > $searchTrjIter.weight".tmp" && mv $searchTrjIter.weight".tmp" $searchTrjIter.weight
            awk -v x="${igap}" -F" "  '{$1/=x;print}' $killTrjIter.weight \
                          > $killTrjIter.weight".tmp" && mv $killTrjIter.weight".tmp" $killTrjIter.weight
          fi

          if ! [ $neural_nets = true ]; then 
            cat $searchTrjIter >> $searchTrj
          else
            cat $searchTrjIter.1 >> $searchTrj.1
            cat $searchTrjIter.2 >> $searchTrj.2
          fi
          cat $searchTrjIter.weight >> $searchTrj.weight
        fi

        if ! [ $neural_nets = true ]; then
          rm -f $searchTrjIter $killTrjIter.weight $searchTrjIter.weight
        else
          rm -f $searchTrjIter.1 $searchTrjIter.2 $searchTrjIter.weight
        fi

      done
    done

    if [ $startdiff = true ]; then
      searchPolicy=$policyDir/searchPolicy.$numPolicy"_keras.json"
      killPolicy=$policyDir/killPolicy.$numPolicy"_keras.json"
      selNormFile=$policyDir/searchPolicy.$numPolicy.norm.dat
      pruNormFile=$policyDir/killPolicy.$numPolicy.norm.dat
      continue
    fi
 
    if [ $startitr = true ]; then
      searchPolicy=$policyDir/searchPolicy.$numPolicy"_keras.json"
      killPolicy=$policyDir/killPolicy.$numPolicy"_keras.json"
      selNormFile=$policyDir/searchPolicy.$numPolicy.norm.dat
      pruNormFile=$policyDir/killPolicy.$numPolicy.norm.dat
      continue
    fi
 
    # Learn a policy after a few examples
    if ! [ -d $scratch/$datDir/$experiment ]; then mkdir -p $scratch/$datDir/$experiment; fi
    
    if ! [ $neural_nets = true ]; then
      searchPolicy=$policyDir/searchPolicy.$numPolicy
      echo "c = $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $searchTrjTrain.weight)"
      c=$(echo "scale=6; $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $searchTrjTrain.weight)" | bc)
      echo "Training search policy $numPolicy with svm c=$c"
      if [ $sel_norm = true ]; then
        selNormFile=$policyDir/searchPolicy.$numPolicy.norm.dat
        python pyscripts/normdata.py --i $searchTrjTrain --o $searchTrjTrain.norm --n $selNormFile
        bin/train-w -c $c -W $searchTrjTrain.weight $searchTrjTrain.norm $searchPolicy
        bin/predict $searchTrjTrain.norm $searchPolicy $scratch/$datDir/$experiment/pred
      else
        bin/train-w -c $c -W $searchTrjTrain.weight $searchTrjTrain $searchPolicy
        bin/predict $searchTrjTrain $searchPolicy $scratch/$datDir/$experiment/pred
      fi

      killPolicy=$policyDir/killPolicy.$numPolicy
      echo "c = $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $killTrjTrain.weight)"
      c=$(echo "scale=6; $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $killTrjTrain.weight)" | bc)
      if [ $numPolicy == 0 ]; then w=1; else w=$svmw; fi
      echo "Training node kill policy $numPolicy with svm c=$c and w-1=$w"
      if [ $pru_norm = true ]; then
        pruNormFile=$policyDir/killPolicy.$numPolicy.norm.dat
        python pyscripts/normdata.py --i $killTrjTrain --o $killTrjTrain.norm --n $pruNormFile
        bin/train-w -c $c -w-1 $w -W $killTrjTrain.weight $killTrjTrain.norm $killPolicy
        bin/predict $killTrjTrain.norm $killPolicy $scratch/$datDir/$experiment/pred
        python pyscripts/lin_model_prec_recall.py --p $scratch/$datDir/$experiment/pred --t $killTrjTrain.norm
        python pyscripts/feat_var.py --i $killTrjTrain.norm --m $scratch/$datDir/$experiment/mean.$numPolicy.txt --v $scratch/$datDir/$experiment/variance.$numPolicy.txt
      else
        bin/train-w -c $c -w-1 $w -W $killTrjTrain.weight $killTrjTrain $killPolicy
        bin/predict $killTrjTrain $killPolicy $scratch/$datDir/$experiment/pred
        python pyscripts/lin_model_prec_recall.py --p $scratch/$datDir/$experiment/pred --t $killTrjTrain
        python pyscripts/feat_var.py --i $killTrjTrain --m $scratch/$datDir/$experiment/mean.$numPolicy.txt --v $scratch/$datDir/$experiment/variance.$numPolicy.txt
      fi
    else
      searchPolicy=$policyDir/searchPolicy.$numPolicy"_keras.h5"
      jsonSearchPolicy=$policyDir/searchPolicy.$numPolicy"_keras.json"
      echo "Training search policy $numPolicy with neural networks"
      if [ $sel_norm = true ]; then
        selNormFile=$policyDir/searchPolicy.$numPolicy.norm.dat
        echo python pyscripts/normdata.py --i $searchTrjTrain --o $searchTrjTrain.norm --n $selNormFile
        echo python pyscripts/normdata.py --i $searchTrjValid --o $searchTrjValid.norm --p $selNormFile
        #echo python pyscripts/normdata.py --i $searchTrjTest --o $searchTrjTest.norm --p $selNormFile 
        echo python pyscripts/searchnet.py --v $searchTrjValid.norm --t $searchTrjTrain.norm --m $searchPolicy --w $searchTrjTrain.weight --x $searchTrjValid.weight
        python pyscripts/normdata.py --i $searchTrjTrain --o $searchTrjTrain.norm --n $selNormFile 
        #python pyscripts/normdata.py --i $searchTrjValid --o $searchTrjValid.norm --p $selNormFile 
        #python pyscripts/normdata.py --i $searchTrjTest --o $searchTrjTest.norm --p $selNormFile 
        python pyscripts/searchnet.py --v $searchTrjValid.norm --t $searchTrjTrain.norm --m $searchPolicy --w $searchTrjTrain.weight --x $searchTrjValid.weight
      else
        python pyscripts/searchnet.py --v $searchTrjValid --t $searchTrjTrain --u $searchTrjTest --m $searchPolicy --w $searchTrjTrain.weight --x $searchTrjValid.weight
      fi
      #searchPolicy=$policyDir/searchPolicy.$numPolicy"_keras.json"

    fi
    numPolicy=$((numPolicy+1)) 
  done
done


#rm $trjDir/*

