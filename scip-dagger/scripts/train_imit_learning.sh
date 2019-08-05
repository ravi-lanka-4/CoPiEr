#!/bin/bash

set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -x <suffix> -p <num_passes> -n <neural_nets_bool> -c <svm_c> -w <svm_w> -e <experiment> -r <restriced_level> -l <low> -u <high> -i <increment> -f <prob_feats_dir> -a <prune_norm_bool> -b <sel_norm_bool>"
  echo "Example: scripts/train_bb.sh -d data/psulu/ts_10/ -p 2 -n 1 -x .lp -l 10 -u 14 -i 1 -f feats/psulu"
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

while getopts ":hd:p:n:c:e:w:tx:m:r:l:u:i:f:a:b:" arg; do
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

datDir="$data"
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
    datDir=$(echo $datDir | sed -e 's/'$((j-$increment))'/'$j'/g')
  fi
  solDir="solution/$datDir"
  scratch="/tmp/subrahma/tmp/"

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

  policyDir=policy/$datDir/train/$experiment
  policySol=$scratch/policysol/$datDir/train/$experiment
  if ! [ -d $policyDir ]; then mkdir -p $policyDir; fi
  if ! [ -d $policySol ]; then mkdir -p $policySol; fi

  searchPolicy=""
  killPolicy=""
  numPolicy=0
  selNormFile=""
  pruNormFile=""

  split_pre=("train" "valid" "test")

  searchTrjTrain=$trjDir/"train"/"search.trj"
  killTrjTrain=$trjDir/"train"/"kill.trj"
  searchTrjValid=$trjDir/"valid"/"search.trj"
  killTrjValid=$trjDir/"valid"/"kill.trj"
  searchTrjTest=$trjDir/"test"/"search.trj"
  killTrjTest=$trjDir/"test"/"kill.trj"

  # Remove trajectories if they already exist
  for item in ${split_pre[*]}; do
    killTrj=$trjDir/$item/"kill.trj"
    if [ -e $killTrj ]; then rm $killTrj; echo "rm $killTrj"; fi
    killTrjWeight=$killTrj.weight
    if [ -e $killTrjWeight ]; then rm $killTrjWeight; echo "rm $killTrjWeight"; fi
    searchTrj=$trjDir/$item/"search.trj"
    if [ -e $searchTrj ]; then rm $searchTrj; echo "rm $searchTrj"; fi
    searchTrjWeight=$searchTrj.weight
    if [ -e $searchTrjWeight ]; then rm $searchTrjWeight; echo "rm $searchTrjWeight"; fi
  done

  for i in `seq 1 $numPasses`; do
    for item in ${split_pre[*]}; do
      curDatDir=$datDir/$item
      curSolDir=$solDir/$item
      curPolicySol=$policySol/$item
      curTrjDir=$trjDir/$item
      searchTrj=$curTrjDir/"search.trj"
      killTrj=$curTrjDir/"kill.trj"
      for prob in `ls $curDatDir`; do
        echo $prob
        base=`sed "s/$suffix//g" <<< $prob`
        prob=$curDatDir/$prob
        sol=$curSolDir/$base.sol

        killTrjIter=$curTrjDir/$base.kill.trj.$numPolicy
        searchTrjIter=$curTrjDir/$base.search.trj.$numPolicy
        probfeatsIter=""
        if ! [ -z $prob_feats ]; then
          probfeatsIter=$prob_feats/$item/feats${base:5}.txt
        fi
        echo "$prob_feats"

        if ! [ -d $curPolicySol ]; then mkdir -p $curPolicySol; fi
        if ! [ -d $curTrjDir ]; then mkdir -p $curTrjDir; fi

        if [ -z $searchPolicy ]; then
          # First round, no policy yet
          echo "Gathering first iteration trajectory data"
          # Oracle mode
          echo bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodesel oracle --nodeseltrj $searchTrjIter \
                              --probfeats "$probfeatsIter" --nodepru oracle --nodeprutrj $killTrjIter 
          bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodesel oracle --nodeseltrj $searchTrjIter \
                              --probfeats "$probfeatsIter" --nodepru oracle --nodeprutrj $killTrjIter 
          if ! [ $neural_nets = true ]; then 
            cat $searchTrjIter >> $searchTrj
          else
            cat $searchTrjIter.1 >> $searchTrj.1
            cat $searchTrjIter.2 >> $searchTrj.2
          fi
          cat $searchTrjIter.weight >> $searchTrj.weight
          cat $killTrjIter >> $killTrj
          cat $killTrjIter.weight >> $killTrj.weight
        else
          # Search with policy 
          echo "Gathering trajectory data with $policy"   
          echo bin/scipdagger --vbc $trjDir/vbc.txt -r $freq -s scip.set -f $prob \
                            --nodesel dagger $searchPolicy --nodeseltrj $searchTrjIter \
                            --nodepru dagger $killPolicy --nodeprutrj $killTrjIter \
                            -o $sol --probfeats "$probfeatsIter" --pnorm "$pruNormFile" --snorm "$selNormFile"
          bin/scipdagger --vbc $trjDir/vbc.txt -r $freq -s scip.set -f $prob \
                            --nodesel dagger $searchPolicy --nodeseltrj $searchTrjIter \
                            --nodepru dagger $killPolicy --nodeprutrj $killTrjIter \
                            -o $sol --probfeats "$probfeatsIter" --pnorm "$pruNormFile" --snorm "$selNormFile"

          if ! [ $neural_nets = true ]; then 
            cat $searchTrjIter >> $searchTrj
          else
            cat $searchTrjIter.1 >> $searchTrj.1
            cat $searchTrjIter.2 >> $searchTrj.2
          fi
          cat $searchTrjIter.weight >> $searchTrj.weight
          cat $killTrjIter >> $killTrj
          cat $killTrjIter.weight >> $killTrj.weight
        fi
        if ! [ $neural_nets = true ]; then
          rm $killTrjIter $searchTrjIter $killTrjIter.weight $searchTrjIter.weight
        else
          rm $killTrjIter $searchTrjIter.1 $searchTrjIter.2 $killTrjIter.weight $searchTrjIter.weight
        fi
      done
    done

    # Learn a policy after a few examples
    if ! [ -d $scratch/$datDir/$experiment ]; then mkdir -p $scratch/$datDir/$experiment; fi
    
    if ! [ $neural_nets = true ]; then
      searchPolicy=$policyDir/searchPolicy.$numPolicy
      echo "c = $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $searchTrjTrain.weight)"
      c=$(echo "scale=6; $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $searchTrjTrain.weight)" | bc)
      echo "Training search policy $numPolicy with svm c=$c"
      if [ $sel_norm = true ]; then
        selNormFile=$policyDir/searchPolicy.$numPolicy.norm.dat
        python pyscripts/normdata.py --i $searchTrjTrain --o $searchTrjTrain.norm --n $selNormFile --f $numfeats
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
      searchPolicy=$policyDir/searchPolicy.$numPolicy.h5
      echo "Training search policy $numPolicy with neural networks"
      if [ $sel_norm = true ]; then
        selNormFile=$policyDir/searchPolicy.$numPolicy.norm.dat
        python pyscripts/normdata.py --i $searchTrjTrain --o $searchTrjTrain.norm --n $selNormFile --f $numfeats
        python pyscripts/normdata.py --i $searchTrjValid --o $searchTrjValid.norm --p $selNormFile --f $numfeats
        python pyscripts/normdata.py --i $searchTrjTest --o $searchTrjTest.norm --p $selNormFile --f $numfeats
        python pyscripts/searchnet.py --v $searchTrjValid.norm --t $searchTrjTrain.norm --u $searchTrjTest.norm --m $searchPolicy --w $searchTrjTrain.weight --x $searchTrjValid.weight
      else
        python pyscripts/searchnet.py --v $searchTrjValid --t $searchTrjTrain --u $searchTrjTest --m $searchPolicy --w $searchTrjTrain.weight --x $searchTrjValid.weight
      fi

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

