#!/bin/bash

set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -x <suffix> -p <num_passes> -n <neural_nets_bool> -c <svm_c> -w <svm_w> -e <experiment> -r <restriced_level> -l <low> -u <high> -i <increment> -j <prune_noise> -f <prob_feats_dir> -a <prune_norm_bool>"
  echo "Example: scripts/train_bb.sh -d data/psulu/ts_10/ -p 2 -n 1 -x .lp -l 10 -u 14 -i 1 -f feats/psulu"
}

suffix=".lp"
freq=1
low=-1
high=-1
increment=1
neural_nets=false
pru_noise=0
prob_feats=""
pru_norm=false

while getopts ":hd:p:n:c:e:w:tx:m:r:l:u:i:j:f:a:" arg; do
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
    n)
      if [ ${OPTARG} > 0 ]; then
        neural_nets=true
      else
        neural_nets=false
      fi
      echo "neural nets: $neural_nets"
      ;;
    c)
      svmc=${OPTARG}
      echo "svm parameter c: $svmc"
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

datDir="$data"
prevDatDir="$data"
prevKillPolicy=""
prevPruNormFile=""
numfeats=0
# if we running experiments on psulu dataset and problem features specified, 
# read in number of problem features
if [ ${datDir:5:5} == "psulu" ] && ! [ -z $prob_feats ]; then
  numfeats=$(($(head -n 1 feats/psulu/train/feats1.txt)))
  echo "$numfeats"
fi

for (( j=$low; j<=$high; j=$j+$increment )); do
  if [ "$low" \< "0" ]; then  
    datDir="$data"
  else
    prevDatDir=$(echo $datDir)
    datDir=$(echo $datDir | sed -e 's/'$((j-$increment))'/'$j'/g')
  fi
  solDir="solution/$datDir"
  scratch="/tmp/azzhao/tmp/"

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
  vbcF=$trjDir/"vbc.tmp"

  split_pre=("train" "valid" "test")
  killTrjTrain=$trjDir/"train"/"kill.trj"
  killTrjValid=$trjDir/"valid"/"kill.trj"
  killTrjTest=$trjDir/"test"/"kill.trj"
  # Remove trajectories if they already exist
  for item in ${split_pre[*]}; do
    killTrj=$trjDir/$item/"kill.trj"
    if [ -e $killTrj ]; then rm $killTrj; echo "rm $killTrj"; fi
    killTrjWeight=$killTrj.weight
    if [ -e $killTrjWeight ]; then rm $killTrjWeight; echo "rm $killTrjWeight"; fi
  done

  policyDir=policy/$datDir/train/$experiment
  policySol=$scratch/policysol/$datDir/train/$experiment
  if ! [ -d $policyDir ]; then mkdir -p $policyDir; fi
  if ! [ -d $policySol ]; then mkdir -p $policySol; fi


  prevKillPolicy=$(echo $killPolicy)
  killPolicy=""
  numPolicy=0
  prevPruNormFile=$(echo $pruNormFile)
  pruNormFile=""
  pru_noise_iter=0

  for i in `seq 1 $numPasses`; do
    # No prune noise when training on problems of initial size
    if [ $j -eq $low ]; then
      pru_noise_iter=0
    # When scaling up, for first pass, we use specified prune noise and for
    # every subsequent pass, we divide prune noise by 10
    else
      if [ $i -eq 1 ]; then
        pru_noise_iter=$pru_noise
      else
        pru_noise_iter=$(echo $pru_noise_iter/10 | bc -l)
      fi
    fi
    for item in ${split_pre[*]}; do
      curDatDir=$datDir/$item
      curSolDir=$solDir/$item
      curPolicySol=$policySol/$item
      curTrjDir=$trjDir/$item
      killTrj=$curTrjDir/"kill.trj"
      for prob in `ls $curDatDir`; do
        echo $prob
        base=`sed "s/$suffix//g" <<< $prob`
        prob=$curDatDir/$prob
        sol=$curSolDir/$base.sol

        killTrjIter=$curTrjDir/$base.kill.trj.$numPolicy
        probfeatsIter=""
        if ! [ -z $prob_feats ]; then
          probfeatsIter=$prob_feats/$item/feats${base:5}.txt
        fi
        echo "$prob_feats"

        if ! [ -d $curPolicySol ]; then mkdir -p $curPolicySol; fi
        if ! [ -d $curTrjDir ]; then mkdir -p $curTrjDir; fi

        echo "pru_noise_iter: $pru_noise_iter"
        if [ -z $killPolicy ]; then
          # First round, no policy yet
          echo "Gathering first iteration trajectory data"
          if [ "$datDir" != "$prevDatDir" ]; then
            # If leftover solutions from previous runs, remove them
            if [ -d $curPolicySol/$base/$i/ ]; then rm -r $curPolicySol/$base/$i/; echo "rm -r $curPolicySol/$base/$i/"; fi
            if ! [ -d $curPolicySol/$base/$i ]; then mkdir -p $curPolicySol/$base/$i; fi

            # Produce the solution file in Policy mode 
            echo bin/scipdagger -r $freq -s scip.set -f $prob \
                                --nodepru policy $prevKillPolicy --allsols_write $curPolicySol/$base/$i/ --sol $curPolicySol/$base.sol \
                                -j $pru_noise_iter --probfeats "$probfeatsIter" --pnorm "$prevPruNormFile" 
            bin/scipdagger -r $freq -s scip.set -f $prob \
                                --nodepru policy $prevKillPolicy --allsols_write $curPolicySol/$base/$i/ --sol $curPolicySol/$base.sol \
                                -j $pru_noise_iter --probfeats "$probfeatsIter" --pnorm "$prevPruNormFile" 

            echo bin/scipdagger -r $freq -s scip.set -f $prob -o $curPolicySol/$base.sol --nodesel oracle \
                                --probfeats "$probfeatsIter" --nodepru oracle --nodeprutrj $killTrjIter
            bin/scipdagger -r $freq -s scip.set -f $prob -o $curPolicySol/$base.sol --nodesel oracle \
                                --probfeats "$probfeatsIter" --nodepru oracle --nodeprutrj $killTrjIter 

            # use the solution file to produce self imitation traces
            # echo bin/scipdagger -r $freq -s scip.set -f $prob \
            #                     --nodepru dagger $prevKillPolicy \
            #                     --nodeprutrj $killTrjIter \
            #                     -g 0.1 --vbc $curTrjDir/vbc.txt --allsols_prefix $curPolicySol/$base/$i/ -o $curPolicySol/$base.sol \
            #                     -j $pru_noise_iter --probfeats "$probfeatsIter" --pnorm "$prevPruNormFile"  
            # bin/scipdagger -r $freq -s scip.set -f $prob \
            #                     --nodepru dagger $prevKillPolicy \
            #                     --nodeprutrj $killTrjIter \
            #                     -g 0.1 --vbc $curTrjDir/vbc.txt --allsols_prefix $curPolicySol/$base/$i/ -o $curPolicySol/$base.sol \
            #                     -j $pru_noise_iter --probfeats "$probfeatsIter" --pnorm "$prevPruNormFile"  

          else
            # Oracle mode
            echo bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodesel oracle \
                                --probfeats "$probfeatsIter" --nodepru oracle --nodeprutrj $killTrjIter 
            bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodesel oracle \
                                --probfeats "$probfeatsIter" --nodepru oracle --nodeprutrj $killTrjIter 
          fi
          cat $killTrjIter >> $killTrj
          cat $killTrjIter.weight >> $killTrj.weight
        else
          # Search with policy 
          echo "Gathering trajectory data with $policy"
          # If leftover solutions from previous runs, remove them
          if [ -d $curPolicySol/$base/$i/ ]; then rm -r $curPolicySol/$base/$i/; echo "rm -r $curPolicySol/$base/$i/"; fi
          if ! [ -d $curPolicySol/$base/$i ]; then mkdir -p $curPolicySol/$base/$i; fi

          # Produce the solution file in Policy mode 
          echo bin/scipdagger -r $freq -s scip.set -f $prob \
                              --nodepru policy $killPolicy --allsols_write $curPolicySol/$base/$i/ --sol $curPolicySol/$base.sol \
                              -j $pru_noise_iter --probfeats "$probfeatsIter" --pnorm "$pruNormFile" 
          bin/scipdagger -r $freq -s scip.set -f $prob \
                              --nodepru policy $killPolicy --allsols_write $curPolicySol/$base/$i/ --sol $curPolicySol/$base.sol \
                              -j $pru_noise_iter --probfeats "$probfeatsIter" --pnorm "$pruNormFile" 
          
          opt_sol=""
          if [ $j -eq $low ]; then
            opt_sol=$sol
          else
            opt_sol=$curPolicySol/$base.sol
          fi

          # use the solution file to produce self imitation traces          
          echo bin/scipdagger --vbc $trjDir/vbc.txt -r $freq -s scip.set -f $prob \
                            --nodepru dagger $killPolicy --nodeprutrj $killTrjIter \
                            --allsols_prefix $curPolicySol/$base/$i/ -o $opt_sol -j $pru_noise_iter \
                            -g 0.1 --probfeats "$probfeatsIter" --pnorm "$pruNormFile"  
          bin/scipdagger --vbc $trjDir/vbc.txt -r $freq -s scip.set -f $prob \
                            --nodepru dagger $killPolicy --nodeprutrj $killTrjIter \
                            --allsols_prefix $curPolicySol/$base/$i/ -o $opt_sol -j $pru_noise_iter \
                            -g 0.1 --probfeats "$probfeatsIter" --pnorm "$pruNormFile" 

          cat $killTrjIter >> $killTrj
          cat $killTrjIter.weight >> $killTrj.weight
        fi
        if ! [ $neural_nets = true ]; then
          rm $killTrjIter $killTrjIter.weight 
        else
          rm $killTrjIter $killTrjIter.weight
        fi
      done
    done

    # Learn a policy after a few examples
    if ! [ -d $scratch/$datDir/$experiment ]; then mkdir -p $scratch/$datDir/$experiment; fi
    
    if ! [ $neural_nets = true ]; then
      killPolicy=$policyDir/killPolicy.$numPolicy
      echo "c = $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $killTrjTrain.weight)"
      c=$(echo "scale=6; $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $killTrjTrain.weight)" | bc)
      if [ $numPolicy == 0 ]; then w=1; else w=$svmw; fi
      echo "Training node kill policy $numPolicy with svm c=$c and w-1=$w"
      if [ $pru_norm = true ]; then
        pruNormFile=$policyDir/killPolicy.$numPolicy.norm.dat
        python pyscripts/normdata.py --i $killTrjTrain --o $killTrjTrain.norm --n $pruNormFile --f $numfeats
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
      killPolicy=$policyDir/killPolicy.$numPolicy.h5
      echo "Training kill policy $numPolicy with neural networks"
      if [ $pru_norm = true ]; then
        pruNormFile=$policyDir/killPolicy.$numPolicy.norm.dat
        python pyscripts/normdata.py --i $killTrjTrain --o $killTrjTrain.norm --n $pruNormFile --f $numfeats
        python pyscripts/normdata.py --i $killTrjValid --o $killTrjValid.norm --p $pruNormFile --f $numfeats
        python pyscripts/normdata.py --i $killTrjTest --o $killTrjTest.norm --p $pruNormFile --f $numfeats
        python pyscripts/prunenet.py --v $killTrjValid.norm --t $killTrjTrain.norm --u $killTrjTest.norm --m $killPolicy --w $killTrjTrain.weight --x $killTrjValid.weight
        python pyscripts/feat_var.py --i $killTrjTrain.norm --m $scratch/$datDir/$experiment/mean.$numPolicy.txt --v $scratch/$datDir/$experiment/variance.$numPolicy.txt
      else
        python pyscripts/prunenet.py --v $killTrjValid --t $killTrjTrain --u $killTrjTest --m $killPolicy --w $killTrjTrain.weight --x $killTrjValid.weight
        python pyscripts/feat_var.py --i $killTrjTrain --m $scratch/$datDir/$experiment/mean.$numPolicy.txt --v $scratch/$datDir/$experiment/variance.$numPolicy.txt
      fi
    fi
    numPolicy=$((numPolicy+1)) 
  done
done


#rm $trjDir/*

