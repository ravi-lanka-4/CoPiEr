#!/bin/bash
usage() {
  echo "Usage: $0 -d <data_path_under_dat> -e <experiment> -n <policy_id> -x <suffix>"
}

nnodesflag=false
while getopts ":hd:e:x:c:w:n:p:" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      ;;
    c)
      c=${OPTARG}
      ;;
    w)
      w=${OPTARG}
      ;;
    n)
      n=${OPTARG}
      ;;
    e)
      experiment=${OPTARG}
      ;;
    x)
      suffix=${OPTARG}
      ;;
    p)
      if [ ${OPTARG} > 0 ]; then
        nnodesflag=true
      else
        nnodesflag=false
      fi
      echo "neural nets: $neural_nets"
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

read c w <<< ${experiment//[^0-9]/" "}

datDir=$data
resultDir=/tmp/results/
output=$resultDir/$data/$experiment/stats
nnodesout=$resultDir/$data/$experiment/nnodes.txt
if [ -e $nnodesout ]; then rm $nnodesout; echo "rm $nnodesout"; fi

echo $output
echo "getting stats of $experiment for $data (suffix=$suffix)"
# print header
printf "%-20s %-6s %-6s %-10s %-10s %-10s %-5s %-5s\n" \
"problem" "total_nnodes" "time" "DB" "PB" "opt" "ogap" "igap" > $output

re='^[0-9]*([.][0-9]+)?$'
fail=0
echo $datDir
for prob in `ls $datDir`; do
  base=`sed "s/$suffix//g" <<< $prob`

  # read scip log 
  log=$resultDir/$data/$experiment/$base.log
  inf=$(grep "infeasible" $log | wc -l)
  if ! [[ "$inf" -eq "0" ]]; then
     fail=$((fail+1))
     if [ $nnodesflag = true ]; then
       printf "\n" >> $nnodesout
     fi
  continue
  fi
  inf=$(grep "Gap" $log -m 1 | grep "infinite" | wc -l)
  if ! [[ "$inf" -eq "0" ]]; then
    echo $log
    fail=$((fail+1))
    if [ $nnodesflag = true ]; then
      printf "\n" >> $nnodesout
    fi
    continue
  fi  
  sol=solution/$data/$base.sol
  if ! [ -e $log ]; then
    echo "missing $base.log"
    if [ $nnodesflag = true ]; then
      printf "\n" >> $nnodesout
    fi
    continue
  fi
  # second line is variables/constraints in the presolved problem
  #nvars=$(grep "Variables" $log | head -n 1 | sed "s/\s\+/ /g" | cut -d' ' -f4) 
  #nconstrs=$(grep "Constraints" $log | head -n 1 | sed "s/\s\+/ /g" | cut -d' ' -f4) 

  #Number of nodes (including restarts)
  echo "nodes"
  nnodes=$(grep "nodes (total)" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  nnodes=$(echo $nnodes | awk '{sub(/ .*/,x)}1')
  nnodes=$(echo $nnodes | sed 's/[eE]+*/*10^/g')
  nnodes=$(echo $nnodes | sed 's/+//g' | bc -l)

  # Total time
  echo "time"
  time=$(grep "Solving Time" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  time=$(echo $time | awk '{sub(/ .*/,x)}1')
  time=$(echo $time | sed 's/[eE]+*/*10^/g')
  time=$(echo $time | sed 's/+//g' | bc -l)

  # Dual bound
  echo "Dual Bound"
  db=$(grep "Dual Bound" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  db=$(echo $db | awk '{sub(/ .*/,x)}1')    
  db=$(echo $db | sed 's/[eE]+*/*10^/g')
  db=$(echo $db | sed 's/+//g' | bc -l)

  # Primal Bound
  echo "Primal Bound"
  pb=$(grep "Primal Bound" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  pb=$(echo $pb | awk '{sub(/ .*/,x)}1')    
  pb=$(echo $pb | sed 's/[eE]+*/*10^/g')
  pb=$(echo $pb | sed 's/+//g' | bc -l)

  # Integrality gap
  echo "Gap"
  igap=$(grep "Gap" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  igap=$(echo $igap | awk '{sub(/ .*/,x)}1')    
  igap=$(echo $igap | sed 's/[eE]+*/*10^/g')  
  igap=$(echo $igap | sed 's/+//g' | bc -l)  

  echo "Objective"
  opt=$(grep "Objective" $sol | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  opt=$(echo $opt | awk '{sub(/ .*/,x)}1')
  opt=$(echo $opt | sed 's/[eE]+*/*10^/g')
  opt=$(echo $opt | sed 's/+//g' | bc -l)

  ogap=$(echo "$pb $opt" | awk 'function abs(x){return ((x < 0.0) ? -x : x)} {print abs(($1-$2)/$2)}')
  ogap=$(echo $ogap | awk '{sub(/ .*/,x)}1')
  ogap=$(echo $ogap | sed 's/[eE]+*/*10^/g')
  ogap=$(echo $ogap | sed 's/+//g' | bc -l)

  if [ $nnodesflag = true ]; then
    thres=$(echo "$ogap > 1.75" | bc)
    if [ $thres -eq 1 ]; then
       fail=$((fail+1))
       continue
    fi
  fi

  if [ $nnodesflag = true ]; then
    printf "%-6d\n" $nnodes >> $nnodesout
  fi
  printf "%-20s %-6d %-6.2f %-10.2f %-10.2f %-10.2f %-5.2f %-5.2f\n" \
  $base $nnodes $time $db $pb $opt $ogap $igap >> $output
done

# compute average
tail -n +2 $output | awk -v fail=$fail '{
nnodes += $2;
time += $3;
db += $4;
pb += $5;
opt += $6
ogap += $7;
igap += $8;
}
END {
printf "%-20s %-6d %-6.2f %-10.2f %-10.2f %-10.2f %-5.2f %-5.2f %-4d\n", "average", nnodes/NR, time/NR, db/NR, pb/NR, opt/NR, ogap/NR, igap/NR, fail;
}' >> $output

echo "saved in $output"
