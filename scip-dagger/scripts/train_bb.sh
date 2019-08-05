#!/bin/bash

set -e

usage() {
  echo "Usage: $0 -d <data_path_under_dat> -x <suffix> -p <num_passes> -c <svm_c> -w <svm_w> -e <experiment> -r <restriced_level> -l <low> -h <high>"
  echo "Example: scripts/train_bb.sh -d data/psulu/ts_10/train/ -p 2 -c 2 -w 8 -x .lp -l 10 -h 14 -l 10 -u 14"
}

suffix=".lp"
freq=1
low=-1
high=-1

while getopts ":hd:p:n:c:e:w:tx:m:r:l:u:" arg; do
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
prevSearchPolicy=""
prevKillPolicy=""
for (( j=$low; j<=$high; j++ )); do
  if [ "$low" \< "0" ]; then  
    datDir="$data"
  else
    prevDatDir=$(echo $datDir)
    datDir=$(echo $datDir | sed -e 's/'$((j-1))'/'$j'/g')
  fi
  solDir="solution/$datDir"
  scratch="/tmp/"

  echo $datDir

  if [ -z $experiment ]; then
     experiment=c${svmc}w${svmw}
  fi

  trjDir=$scratch/$experiment
  if ! [ -d $trjDir ]; then mkdir -p $trjDir; fi
  searchTrj=$trjDir/"search.trj"
  killTrj=$trjDir/"kill.trj"
  vbcF=$trjDir/"vbc.tmp"

  # We need to append to these trj
  if [ -e $searchTrj ]; then rm $searchTrj; echo "rm $searchTrj"; fi
  if [ -e $killTrj ]; then rm $killTrj; echo "rm $killTrj"; fi
  if [ -e $searchTrj.weight ]; then rm $searchTrj.weight; fi
  if [ -e $killTrj.weight ]; then rm $killTrj.weight; fi

  policyDir=policy/$datDir/$experiment
  policySol=$scratch/policysol/$datDir/$experiment
  if ! [ -d $policyDir ]; then mkdir -p $policyDir; fi
  if ! [ -d $policySol ]; then mkdir -p $policySol; fi

  prevSearchPolicy=$(echo $searchPolicy)
  prevKillPolicy=$(echo $killPolicy)
  searchPolicy=""
  killPolicy=""
  numPolicy=0

  for i in `seq 1 $numPasses`; do
    for prob in `ls $datDir`; do
      echo $prob
      base=`sed "s/$suffix//g" <<< $prob`
      prob=$datDir/$prob
      sol=$solDir/$base.sol

      killTrjIter=$trjDir/$base.kill.trj.$numPolicy
      searchTrjIter=$trjDir/$base.search.trj.$numPolicy
      traceIter=$trjDir/$base.search.trj.$numPolicy
      allkillTrjIter=$trjDir/$base.allkill.trj.$numPolicy
      allsearchTrjIter=$trjDir/$base.allsearch.trj.$numPolicy

      if [ -z $searchPolicy ]; then
        # First round, no policy yet
        echo "Gathering first iteration trajectory data"
        if [ "$datDir" != "$prevDatDir" ]; then
          # Produce the solution file in Policy mode 
          bin/scipdagger -r $freq -s scip.set -f $prob --nodesel policy $prevSearchPolicy \
                              --nodepru policy $prevKillPolicy --sol $policySol/$base.sol

          # use the solution file to produce self imitation traces
          # bin/scipdagger -r $freq -s scip.set -f $prob -o $policySol/$base.sol \
          #                     --nodesel dagger $prevSearchPolicy --nodepru dagger $prevKillPolicy \
          #                     --nodeseltrj $tmpsearchTrj --nodeprutrj $tmpkillTrj \
          #                     --allseltrj $searchTrjIter --allprutrj $killTrjIter --trace $traceF --vbc $vbcF

          # Oracle mode
          bin/scipdagger -r $freq -s scip.set -f $prob -o $policySol/$base.sol --nodesel oracle \
                              --nodeseltrj $searchTrjIter --nodepru oracle --nodeprutrj $killTrjIter


        else
          # Oracle mode
          bin/scipdagger -r $freq -s scip.set -f $prob -o $sol --nodesel oracle --nodeseltrj $searchTrjIter \
                              --nodepru oracle --nodeprutrj $killTrjIter
        fi

        cat $searchTrjIter >> $searchTrj
        cat $searchTrjIter.weight >> $searchTrj.weight
        cat $killTrjIter >> $killTrj
        cat $killTrjIter.weight >> $killTrj.weight
      else
        # Search with policy 
        echo "Gathering trajectory data with $policy"
        bin/scipdagger --vbc $trjDir/vbc.txt --trace $trjDir/trace.txt -r $freq -s scip.set -f $prob \
                          -o $sol --nodesel dagger $searchPolicy --nodeseltrj $searchTrjIter \
                          --nodepru dagger $killPolicy --nodeprutrj $killTrjIter
        cat $searchTrjIter >> $searchTrj
        cat $searchTrjIter.weight >> $searchTrj.weight
        cat $killTrjIter >> $killTrj
        cat $killTrjIter.weight >> $killTrj.weight
      fi
      rm $killTrjIter $searchTrjIter $killTrjIter.weight $searchTrjIter.weight
    done

    # Learn a policy after a few examples
    if ! [ -d $scratch/$datDir/$experiment ]; then mkdir -p $scratch/$datDir/$experiment; fi

    searchPolicy=$policyDir/searchPolicy.$numPolicy
    echo "c = $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $searchTrj.weight)"
    c=$(echo "scale=6; $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $searchTrj.weight)" | bc)
    echo "Training search policy $numPolicy with svm c=$c"
    bin/train-w -c $c -W $searchTrj.weight $searchTrj $searchPolicy
    bin/predict $searchTrj $searchPolicy $scratch/$datDir/$experiment/pred

    killPolicy=$policyDir/killPolicy.$numPolicy
    echo "c = $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $killTrj.weight)"
    c=$(echo "scale=6; $svmc/$(awk '{ total+=$1; count+=1} END {print total/count}' $killTrj.weight)" | bc)
    if [ $numPolicy == 0 ]; then w=1; else w=$svmw; fi
    echo "Training node kill policy $numPolicy with svm c=$c and w-1=$w"
    bin/train-w -c $c -w-1 $w -W $killTrj.weight $killTrj $killPolicy
    bin/predict $killTrj $killPolicy $scratch/$datDir/$experiment/pred

    searchPolicy=$policyDir/searchPolicy.$numPolicy
    killPolicy=$policyDir/killPolicy.$numPolicy
    numPolicy=$((numPolicy+1)) 

  done
done


#rm $trjDir/*

