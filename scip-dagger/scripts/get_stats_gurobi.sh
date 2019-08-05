#!/bin/bash
set -e;
usage() {
  echo "Usage: $0 -d <data_path_under_dat> -e <experiment> -x <suffix> -i <inverse>"
}
inverse=false
while getopts ":hd:e:x:c:w:n:i" arg; do
  case $arg in
    h)
      usage
      exit 0
      ;;
    d)
      data=${OPTARG%/}
      ;;
    e)
      experiment=${OPTARG}
      ;;
    x)
      suffix=${OPTARG}
      ;;
    i)
      inverse=true
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

datDir=$data
resultDir=/tmp/results/
output=$resultDir/$data/$experiment/stats

echo "getting stats of $experiment for $data (suffix=$suffix)"
# print header
printf "%-20s %-6s %-6s %-10s %-10s %-10s %-5s %-5s\n" \
"problem" "nnodes" "time" "DB" "PB" "opt" "ogap" "igap" > $output

re='^[0-9]*([.][0-9]+)?$'
fail=0
for prob in `ls $datDir`; do
  base=`sed "s/$suffix//g" <<< $prob`
  # read scip log 
  log=$resultDir/$data/$experiment/$base.log
  sol=solution/$data/$base.sol
  if ! [ -e $log ]; then
    echo "missing $base.log"
    continue
  fi

  #Number of nodes and time
  nodestime=$(grep "Explored" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  nodestime=($nodestime)
  nnodes=${nodestime[0]}
  time=${nodestime[2]}

  # Best Objective
  bestobjbound=$(grep "Best objective" $log | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  bestobjbound=($bestobjbound)
  # Primal
  pb=${bestobjbound[0]}
  pb=$(echo $pb | sed 's/[eE]+*/*10^/g' | bc -l)
  # Dual
  db=${bestobjbound[1]}
  db=$(echo $db | sed 's/[eE]+*/*10^/g' | bc -l)
  # Gap
  igap=${bestobjbound[2]}
  igap=$(echo $igap | sed 's/[eE]+*/*10^/g' | bc -l)

  # Optimal bound
  opt=$(grep "Objective" $sol | grep -Eo '[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?' | tr '\n' ' '; echo "")
  opt=$(echo $opt | awk '{sub(/ .*/,x)}1')
  opt=$(echo $opt | sed 's/[eE]+*/*10^/g')
  opt=$(echo $opt | sed 's/+//g' | bc -l)

  if [[ -z $igap ]] ; then
    echo "Not a number"
    fail=$((fail+1))
    continue
  fi

  if [[ -z $db ]] ; then
    echo "Not a number"
    fail=$((fail+1))
  fi

  if [[ -z $pb ]] ; then
    echo "Not a number"
    fail=$((fail+1))
  else
    # because scip convert to min, negate gurobi's result
    if [ $inverse == true ]; then
      db=$(echo $db | awk '{if ($1 < 0) print -1*$1; else print $1}')
      pb=$(echo $pb | awk '{if ($1 < 0) print -1*$1; else print $1}')
    fi

    ogap=$(echo "$pb $opt" | awk 'function abs(x){return ((x < 0.0) ? -x : x)} $2 != 0 {print abs(($1-$2)/$2)} $2 = 0 {print 0}')



    #echo $base $nnodes $time $db $pb $opt $ogap $igap
    printf "%-20s %-6d %-6.2f %-10.2f %-10.2f %-10.2f %-5.2f %-5.2f\n" \
    $base $nnodes $time $db $pb $opt $ogap $igap >> $output
  fi
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
