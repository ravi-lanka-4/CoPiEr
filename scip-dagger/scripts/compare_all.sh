#!/bin/bash

set -e

low=10
high=14
increment=1
policy=0
count=1
folder="test"

for (( i=0; i<$count; i=$i+$increment )); do
  for (( j=$low; j<=$high; j=$j+$increment )); do
    ./scripts/compare.sh -d ./data/psulu/ts_$j/$folder/ -o ./data/psulu/ts_$j/$folder/ -e nn_psulu -n $policy -m 1 -x .lp -a 1 -b 1
    cp "/tmp/results/data/psulu/ts_$j/$folder/nn_psulu/policy"$policy"/test.result" "/tmp/results/data/psulu/ts_$j/$folder/$folder_ts_$j_$i.txt"
  done
done
